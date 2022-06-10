#include <cassert>

#include <Celerity/PipelineBuilderMacros.hpp>

#include <Gameplay/Events.hpp>
#include <Gameplay/Mortality.hpp>
#include <Gameplay/MortalitySettingsSingleton.hpp>
#include <Gameplay/UnitComponent.hpp>

#include <Math/Scalar.hpp>

#include <Physics/Simulation.hpp>

#include <Shared/Checkpoint.hpp>

#include <Transform/Events.hpp>
#include <Transform/Transform3dComponent.hpp>

namespace Mortality
{
namespace TaskNames
{
static const Emergence::Memory::UniqueString PROCESS_DAMAGE ("Mortality::ProcessDamage");
static const Emergence::Memory::UniqueString PROCESS_CORPSES ("Mortality::ProcessCorpses");
static const Emergence::Memory::UniqueString CLEANUP_UNITS_AFTER_TRANSFORM_REMOVAL (
    "Mortality::CleanupUnitsAfterTransformRemoval");
} // namespace TaskNames

class DamageProcessor final : public Emergence::Celerity::TaskExecutorBase<DamageProcessor>
{
public:
    DamageProcessor (Emergence::Celerity::TaskConstructor &_constructor) noexcept;

    void Execute () noexcept;

private:
    Emergence::Celerity::FetchSingletonQuery fetchTime;
    Emergence::Celerity::FetchSingletonQuery fetchMortalitySettings;
    Emergence::Celerity::EditValueQuery editUnitById;
    Emergence::Celerity::InsertShortTermQuery insertDeathEvent;
    Emergence::Celerity::FetchSequenceQuery fetchDamageEvents;
};

DamageProcessor::DamageProcessor (Emergence::Celerity::TaskConstructor &_constructor) noexcept
    : fetchTime (_constructor.MFetchSingleton (Emergence::Celerity::TimeSingleton)),
      fetchMortalitySettings (_constructor.MFetchSingleton (MortalitySettingsSingleton)),
      editUnitById (_constructor.MEditValue1F (UnitComponent, objectId)),
      insertDeathEvent (_constructor.MInsertShortTerm (DeathEvent)),
      fetchDamageEvents (_constructor.MFetchSequence (DamageEvent))
{
    _constructor.DependOn (Checkpoint::MORTALITY_STARTED);

    // TODO: Temporary. Must be replaced by dependency on damage givers later.
    _constructor.DependOn (Emergence::Physics::Simulation::Checkpoint::SIMULATION_FINISHED);
}

void DamageProcessor::Execute () noexcept
{
    auto timeCursor = fetchTime.Execute ();
    const auto *time = static_cast<const Emergence::Celerity::TimeSingleton *> (*timeCursor);

    auto mortalitySettingsCursor = fetchMortalitySettings.Execute ();
    const auto *mortalitySettings = static_cast<const MortalitySettingsSingleton *> (*mortalitySettingsCursor);
    auto deathEventCursor = insertDeathEvent.Execute ();

    for (auto eventCursor = fetchDamageEvents.Execute ();
         const auto *event = static_cast<const DamageEvent *> (*eventCursor); ++eventCursor)
    {
        assert (event->amount >= 0.0f);
        auto unitCursor = editUnitById.Execute (&event->objectId);

        if (auto *unit = static_cast<UnitComponent *> (*unitCursor); unit && !unit->IsCorpse ())
        {
            unit->health -= event->amount;
            if (unit->health <= 0.0f)
            {
                unit->removeAfterNs = time->fixedTimeNs + mortalitySettings->corpseLifetimeNs;
                auto *deathEvent = static_cast<DeathEvent *> (++deathEventCursor);
                deathEvent->objectId = unit->objectId;
            }
        }
    }
}

class CorpseProcessor final : public Emergence::Celerity::TaskExecutorBase<CorpseProcessor>
{
public:
    CorpseProcessor (Emergence::Celerity::TaskConstructor &_constructor) noexcept;

    void Execute () noexcept;

private:
    Emergence::Celerity::FetchSingletonQuery fetchTime;
    Emergence::Celerity::FetchAscendingRangeQuery fetchCorpsesByRemovalTimer;
    Emergence::Celerity::RemoveValueQuery removeTransformById;
};

CorpseProcessor::CorpseProcessor (Emergence::Celerity::TaskConstructor &_constructor) noexcept
    : fetchTime (_constructor.MFetchSingleton (Emergence::Celerity::TimeSingleton)),
      fetchCorpsesByRemovalTimer (_constructor.MFetchAscendingRange (UnitComponent, removeAfterNs)),
      removeTransformById (_constructor.MRemoveValue1F (Emergence::Transform::Transform3dComponent, objectId))
{
    _constructor.DependOn (TaskNames::PROCESS_DAMAGE);
}

void CorpseProcessor::Execute () noexcept
{
    auto timeCursor = fetchTime.Execute ();
    const auto *time = static_cast<const Emergence::Celerity::TimeSingleton *> (*timeCursor);

    for (auto corpseCursor = fetchCorpsesByRemovalTimer.Execute (nullptr, &time->fixedTimeNs);
         const auto *unit = static_cast<const UnitComponent *> (*corpseCursor); ++corpseCursor)
    {
        auto transformCursor = removeTransformById.Execute (&unit->objectId);
        if (transformCursor.ReadConst ())
        {
            ~transformCursor;
        }
    }
}

class TransformEventProcessor final : public Emergence::Celerity::TaskExecutorBase<TransformEventProcessor>
{
public:
    TransformEventProcessor (Emergence::Celerity::TaskConstructor &_constructor) noexcept;

    void Execute () noexcept;

private:
    Emergence::Celerity::RemoveValueQuery removeUnitById;
    Emergence::Celerity::FetchSequenceQuery fetchTransformRemovedEvents;
};

TransformEventProcessor::TransformEventProcessor (Emergence::Celerity::TaskConstructor &_constructor) noexcept
    : removeUnitById (_constructor.MRemoveValue1F (UnitComponent, objectId)),
      fetchTransformRemovedEvents (
          _constructor.MFetchSequence (Emergence::Transform::Transform3dComponentRemovedFixedEvent))
{
    _constructor.DependOn (TaskNames::PROCESS_CORPSES);
    _constructor.MakeDependencyOf (Checkpoint::MORTALITY_FINISHED);
}

void TransformEventProcessor::Execute () noexcept
{
    for (auto eventCursor = fetchTransformRemovedEvents.Execute ();
         const auto *event =
             static_cast<const Emergence::Transform::Transform3dComponentRemovedFixedEvent *> (*eventCursor);
         ++eventCursor)
    {
        auto unitCursor = removeUnitById.Execute (&event->objectId);
        if (unitCursor.ReadConst ())
        {
            ~unitCursor;
        }
    }
}

void AddToFixedUpdate (Emergence::Celerity::PipelineBuilder &_pipelineBuilder) noexcept
{
    _pipelineBuilder.AddTask (TaskNames::PROCESS_DAMAGE).SetExecutor<DamageProcessor> ();
    _pipelineBuilder.AddTask (TaskNames::PROCESS_CORPSES).SetExecutor<CorpseProcessor> ();
    _pipelineBuilder.AddTask (TaskNames::CLEANUP_UNITS_AFTER_TRANSFORM_REMOVAL).SetExecutor<TransformEventProcessor> ();
}
} // namespace Mortality

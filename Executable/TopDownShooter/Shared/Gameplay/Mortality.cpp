#include <cassert>

#include <Celerity/PipelineBuilderMacros.hpp>

#include <Gameplay/Events.hpp>
#include <Gameplay/MortalComponent.hpp>
#include <Gameplay/Mortality.hpp>
#include <Gameplay/MortalitySettingsSingleton.hpp>

#include <Shared/Checkpoint.hpp>

#include <Transform/Events.hpp>
#include <Transform/Transform3dComponent.hpp>

namespace Mortality
{
namespace TaskNames
{
static const Emergence::Memory::UniqueString PROCESS_DAMAGE ("Mortality::ProcessDamage");
static const Emergence::Memory::UniqueString PROCESS_CORPSES ("Mortality::ProcessCorpses");
static const Emergence::Memory::UniqueString CLEANUP_MORTALS_AFTER_TRANSFORM_REMOVAL (
    "Mortality::CleanupMortalsAfterTransformRemoval");
} // namespace TaskNames

class DamageProcessor final : public Emergence::Celerity::TaskExecutorBase<DamageProcessor>
{
public:
    DamageProcessor (Emergence::Celerity::TaskConstructor &_constructor) noexcept;

    void Execute () noexcept;

private:
    Emergence::Celerity::FetchSingletonQuery fetchTime;
    Emergence::Celerity::FetchSingletonQuery fetchMortalitySettings;
    Emergence::Celerity::EditValueQuery editMortalById;
    Emergence::Celerity::InsertShortTermQuery insertDeathEvent;
    Emergence::Celerity::FetchSequenceQuery fetchDamageEvents;
};

DamageProcessor::DamageProcessor (Emergence::Celerity::TaskConstructor &_constructor) noexcept
    : fetchTime (FETCH_SINGLETON (Emergence::Celerity::TimeSingleton)),
      fetchMortalitySettings (FETCH_SINGLETON (MortalitySettingsSingleton)),
      editMortalById (EDIT_VALUE_1F (MortalComponent, objectId)),
      insertDeathEvent (INSERT_SHORT_TERM (DeathEvent)),
      fetchDamageEvents (FETCH_SEQUENCE (DamageEvent))
{
    _constructor.DependOn (Checkpoint::DAMAGE_FINISHED);
    _constructor.DependOn (Checkpoint::MORTALITY_STARTED);
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
        auto mortalCursor = editMortalById.Execute (&event->objectId);

        if (auto *mortal = static_cast<MortalComponent *> (*mortalCursor); mortal && !mortal->IsCorpse ())
        {
            mortal->health -= event->amount;
            if (mortal->health <= 0.0f)
            {
                mortal->removeAfterNs = time->fixedTimeNs + mortalitySettings->corpseLifetimeNs;
                auto *deathEvent = static_cast<DeathEvent *> (++deathEventCursor);
                deathEvent->objectId = mortal->objectId;
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
    : fetchTime (FETCH_SINGLETON (Emergence::Celerity::TimeSingleton)),
      fetchCorpsesByRemovalTimer (FETCH_ASCENDING_RANGE (MortalComponent, removeAfterNs)),
      removeTransformById (REMOVE_VALUE_1F (Emergence::Transform::Transform3dComponent, objectId))
{
    _constructor.DependOn (TaskNames::PROCESS_DAMAGE);
}

void CorpseProcessor::Execute () noexcept
{
    auto timeCursor = fetchTime.Execute ();
    const auto *time = static_cast<const Emergence::Celerity::TimeSingleton *> (*timeCursor);

    for (auto corpseCursor = fetchCorpsesByRemovalTimer.Execute (nullptr, &time->fixedTimeNs);
         const auto *mortal = static_cast<const MortalComponent *> (*corpseCursor); ++corpseCursor)
    {
        auto transformCursor = removeTransformById.Execute (&mortal->objectId);
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
    Emergence::Celerity::RemoveValueQuery removeMortalById;
    Emergence::Celerity::FetchSequenceQuery fetchTransformRemovedEvents;
};

TransformEventProcessor::TransformEventProcessor (Emergence::Celerity::TaskConstructor &_constructor) noexcept
    : removeMortalById (REMOVE_VALUE_1F (MortalComponent, objectId)),
      fetchTransformRemovedEvents (FETCH_SEQUENCE (Emergence::Transform::Transform3dComponentRemovedFixedEvent))
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
        auto mortalCursor = removeMortalById.Execute (&event->objectId);
        if (mortalCursor.ReadConst ())
        {
            ~mortalCursor;
        }
    }
}

void AddToFixedUpdate (Emergence::Celerity::PipelineBuilder &_pipelineBuilder) noexcept
{
    _pipelineBuilder.AddTask (TaskNames::PROCESS_DAMAGE).SetExecutor<DamageProcessor> ();
    _pipelineBuilder.AddTask (TaskNames::PROCESS_CORPSES).SetExecutor<CorpseProcessor> ();
    _pipelineBuilder.AddTask (TaskNames::CLEANUP_MORTALS_AFTER_TRANSFORM_REMOVAL)
        .SetExecutor<TransformEventProcessor> ();
}
} // namespace Mortality

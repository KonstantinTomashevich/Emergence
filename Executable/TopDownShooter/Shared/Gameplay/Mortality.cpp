#include <cassert>

#include <Celerity/PipelineBuilderMacros.hpp>

#include <Gameplay/EffectConstant.hpp>
#include <Gameplay/Events.hpp>
#include <Gameplay/MortalComponent.hpp>
#include <Gameplay/Mortality.hpp>
#include <Gameplay/MortalitySettingsSingleton.hpp>

#include <Render/ParticleEffectComponent.hpp>

#include <Shared/Checkpoint.hpp>

#include <Transform/Events.hpp>
#include <Transform/Transform3dComponent.hpp>

namespace Mortality
{
namespace TaskNames
{
static const Emergence::Memory::UniqueString PROCESS_DAMAGE ("Mortality::ProcessDamage");
static const Emergence::Memory::UniqueString PROCESS_CORPSES ("Mortality::ProcessCorpses");

static const Emergence::Memory::UniqueString TRIGGER_DEATH_EFFECT ("Mortality::TriggerDeathEffect");
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

    Emergence::Celerity::FetchSequenceQuery fetchDamageEvents;
    Emergence::Celerity::InsertShortTermQuery insertDeathFixedEvent;
    Emergence::Celerity::InsertShortTermQuery insertDeathFixedToNormalEvent;
};

DamageProcessor::DamageProcessor (Emergence::Celerity::TaskConstructor &_constructor) noexcept
    : fetchTime (FETCH_SINGLETON (Emergence::Celerity::TimeSingleton)),
      fetchMortalitySettings (FETCH_SINGLETON (MortalitySettingsSingleton)),
      editMortalById (EDIT_VALUE_1F (MortalComponent, objectId)),

      fetchDamageEvents (FETCH_SEQUENCE (DamageEvent)),
      insertDeathFixedEvent (INSERT_SHORT_TERM (DeathFixedEvent)),
      insertDeathFixedToNormalEvent (INSERT_SHORT_TERM (DeathFixedToNormalEvent))

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

    auto deathFixedEventCursor = insertDeathFixedEvent.Execute ();
    auto deathFixedToNormalEventCursor = insertDeathFixedToNormalEvent.Execute ();

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
                auto *deathFixedEvent = static_cast<DeathFixedEvent *> (++deathFixedEventCursor);
                deathFixedEvent->objectId = mortal->objectId;

                auto *deathFixedToNormalEvent =
                    static_cast<DeathFixedToNormalEvent *> (++deathFixedToNormalEventCursor);
                deathFixedToNormalEvent->objectId = mortal->objectId;
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

void AddToFixedUpdate (Emergence::Celerity::PipelineBuilder &_pipelineBuilder) noexcept
{
    _pipelineBuilder.AddTask (TaskNames::PROCESS_DAMAGE).SetExecutor<DamageProcessor> ();
    _pipelineBuilder.AddTask (TaskNames::PROCESS_CORPSES).SetExecutor<CorpseProcessor> ();

    _pipelineBuilder.AddTask (Emergence::Memory::UniqueString {"Mortality::RemoveMortals"})
        .AS_CASCADE_REMOVER_1F (Emergence::Transform::Transform3dComponentRemovedFixedEvent, MortalComponent, objectId)
        .DependOn (TaskNames::PROCESS_CORPSES)
        .MakeDependencyOf (Checkpoint::MORTALITY_FINISHED);
}

class DeathEffectTrigger final : public Emergence::Celerity::TaskExecutorBase<DeathEffectTrigger>
{
public:
    DeathEffectTrigger (Emergence::Celerity::TaskConstructor &_constructor) noexcept;

    void Execute () noexcept;

private:
    Emergence::Celerity::EditValueQuery editParticleEffectByObjectId;
    Emergence::Celerity::FetchSequenceQuery fetchDeathEvents;
};

DeathEffectTrigger::DeathEffectTrigger (Emergence::Celerity::TaskConstructor &_constructor) noexcept
    : editParticleEffectByObjectId (EDIT_VALUE_1F (ParticleEffectComponent, objectId)),
      fetchDeathEvents (FETCH_SEQUENCE (DeathFixedToNormalEvent))
{
    _constructor.DependOn (Checkpoint::ASSEMBLY_FINISHED);
    _constructor.DependOn (Checkpoint::MORTALITY_STARTED);
    _constructor.MakeDependencyOf (Checkpoint::MORTALITY_FINISHED);
    _constructor.MakeDependencyOf (Checkpoint::RENDER_UPDATE_STARTED);
}

void DeathEffectTrigger::Execute () noexcept
{
    for (auto eventCursor = fetchDeathEvents.Execute ();
         const auto *event = static_cast<const DeathFixedToNormalEvent *> (*eventCursor); ++eventCursor)
    {
        for (auto effectCursor = editParticleEffectByObjectId.Execute (&event->objectId);
             auto *effect = static_cast<ParticleEffectComponent *> (*effectCursor); ++effectCursor)
        {
            if (effect->effectTag == EffectConstant::DEATH_TAG)
            {
                effect->playing = true;
            }
        }
    }
}

void AddToNormalUpdate (Emergence::Celerity::PipelineBuilder &_pipelineBuilder) noexcept
{
    _pipelineBuilder.AddTask (TaskNames::TRIGGER_DEATH_EFFECT).SetExecutor<DeathEffectTrigger> ();
}
} // namespace Mortality

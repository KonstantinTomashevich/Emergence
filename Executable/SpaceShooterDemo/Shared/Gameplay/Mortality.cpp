#include <Celerity/Assembly/Assembly.hpp>
#include <Celerity/PipelineBuilderMacros.hpp>
#include <Celerity/Transform/Events.hpp>
#include <Celerity/Transform/TransformComponent.hpp>
#include <Celerity/Transform/TransformHierarchyCleanup.hpp>

#include <Gameplay/Damage.hpp>
#include <Gameplay/EffectConstant.hpp>
#include <Gameplay/Events.hpp>
#include <Gameplay/MortalComponent.hpp>
#include <Gameplay/Mortality.hpp>

#include <Math/Scalar.hpp>

#include <Render/ParticleEffectComponent.hpp>
#include <Render/Urho3DUpdate.hpp>

namespace Mortality
{
const Emergence::Memory::UniqueString Checkpoint::STARTED {"MortalityStarted"};
const Emergence::Memory::UniqueString Checkpoint::FINISHED {"MortalityFinished"};

namespace TaskNames
{
static const Emergence::Memory::UniqueString PROCESS_LIFETIME ("Mortality::ProcessLifetime");
static const Emergence::Memory::UniqueString PROCESS_DAMAGE ("Mortality::ProcessDamage");
static const Emergence::Memory::UniqueString PROCESS_CORPSES ("Mortality::ProcessCorpses");

static const Emergence::Memory::UniqueString TRIGGER_DEATH_EFFECT ("Mortality::TriggerDeathEffect");
} // namespace TaskNames

class KillerBase
{
public:
    KillerBase (Emergence::Celerity::TaskConstructor &_constructor) noexcept;

protected:
    void Kill (MortalComponent *_mortal) noexcept;

    Emergence::Celerity::FetchSingletonQuery fetchTime;

private:
    Emergence::Celerity::InsertShortTermQuery insertDeathFixedEvent;
    Emergence::Celerity::InsertShortTermQuery insertDeathFixedToNormalEvent;
};

KillerBase::KillerBase (Emergence::Celerity::TaskConstructor &_constructor) noexcept
    : fetchTime (FETCH_SINGLETON (Emergence::Celerity::TimeSingleton)),

      insertDeathFixedEvent (INSERT_SHORT_TERM (DeathFixedEvent)),
      insertDeathFixedToNormalEvent (INSERT_SHORT_TERM (DeathFixedToNormalEvent))
{
}

void KillerBase::Kill (MortalComponent *_mortal) noexcept
{
    auto timeCursor = fetchTime.Execute ();
    const auto *time = static_cast<const Emergence::Celerity::TimeSingleton *> (*timeCursor);

    _mortal->removeAfterNs = time->fixedTimeNs + static_cast<const uint64_t> (_mortal->corpseLifetimeS * 1e9f);
    auto deathFixedEventCursor = insertDeathFixedEvent.Execute ();
    auto *deathFixedEvent = static_cast<DeathFixedEvent *> (++deathFixedEventCursor);
    deathFixedEvent->objectId = _mortal->objectId;

    auto deathFixedToNormalEventCursor = insertDeathFixedToNormalEvent.Execute ();
    auto *deathFixedToNormalEvent = static_cast<DeathFixedToNormalEvent *> (++deathFixedToNormalEventCursor);
    deathFixedToNormalEvent->objectId = _mortal->objectId;
}

class LifetimeProcessor final : public Emergence::Celerity::TaskExecutorBase<LifetimeProcessor>, public KillerBase
{
public:
    LifetimeProcessor (Emergence::Celerity::TaskConstructor &_constructor) noexcept;

    void Execute () noexcept;

private:
    Emergence::Celerity::FetchSequenceQuery fetchMortalAddedEvents;
    Emergence::Celerity::EditValueQuery editMortalById;
    Emergence::Celerity::EditAscendingRangeQuery editOldMortals;
};

LifetimeProcessor::LifetimeProcessor (Emergence::Celerity::TaskConstructor &_constructor) noexcept
    : KillerBase (_constructor),
      fetchMortalAddedEvents (FETCH_SEQUENCE (MortalComponentAddedEvent)),
      editMortalById (EDIT_VALUE_1F (MortalComponent, objectId)),
      editOldMortals (EDIT_ASCENDING_RANGE (MortalComponent, dieAfterNs))
{
    _constructor.DependOn (Checkpoint::STARTED);
}

void LifetimeProcessor::Execute () noexcept
{
    auto timeCursor = fetchTime.Execute ();
    const auto *time = static_cast<const Emergence::Celerity::TimeSingleton *> (*timeCursor);

    for (auto eventCursor = fetchMortalAddedEvents.Execute ();
         const auto *event = static_cast<const MortalComponentAddedEvent *> (*eventCursor); ++eventCursor)
    {
        auto mortalCursor = editMortalById.Execute (&event->objectId);
        if (auto *mortal = static_cast<MortalComponent *> (*mortalCursor);
            mortal && !Emergence::Math::NearlyEqual (0.0f, mortal->maximumLifetimeS))
        {
            mortal->dieAfterNs = time->fixedTimeNs + static_cast<uint64_t> (mortal->maximumLifetimeS * 1e9f);
        }
    }

    for (auto mortalCursor = editOldMortals.Execute (nullptr, &time->fixedTimeNs);
         auto *mortal = static_cast<MortalComponent *> (*mortalCursor); ++mortalCursor)
    {
        if (!mortal->IsCorpse ())
        {
            Kill (mortal);
        }
    }
}

class DamageProcessor final : public Emergence::Celerity::TaskExecutorBase<DamageProcessor>, public KillerBase
{
public:
    DamageProcessor (Emergence::Celerity::TaskConstructor &_constructor) noexcept;

    void Execute () noexcept;

private:
    Emergence::Celerity::EditValueQuery editMortalById;
    Emergence::Celerity::FetchSequenceQuery fetchDamageEvents;
};

DamageProcessor::DamageProcessor (Emergence::Celerity::TaskConstructor &_constructor) noexcept
    : KillerBase (_constructor),
      editMortalById (EDIT_VALUE_1F (MortalComponent, objectId)),
      fetchDamageEvents (FETCH_SEQUENCE (DamageEvent))
{
    _constructor.DependOn (Damage::Checkpoint::FINISHED);
    _constructor.DependOn (TaskNames::PROCESS_LIFETIME);
}

void DamageProcessor::Execute () noexcept
{
    for (auto eventCursor = fetchDamageEvents.Execute ();
         const auto *event = static_cast<const DamageEvent *> (*eventCursor); ++eventCursor)
    {
        EMERGENCE_ASSERT (event->amount >= 0.0f);
        auto mortalCursor = editMortalById.Execute (&event->objectId);

        if (auto *mortal = static_cast<MortalComponent *> (*mortalCursor);
            mortal && !mortal->invincible && !mortal->IsCorpse ())
        {
            mortal->health -= event->amount;
            if (mortal->health <= 0.0f)
            {
                Kill (mortal);
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
      removeTransformById (REMOVE_VALUE_1F (Emergence::Celerity::Transform3dComponent, objectId))
{
    _constructor.DependOn (TaskNames::PROCESS_DAMAGE);
    // Because mortality is de facto last mechanics in the graph, we're taking care or hierarchy cleanup here.
    _constructor.MakeDependencyOf (Emergence::Celerity::TransformHierarchyCleanup::Checkpoint::DETACHED_REMOVAL_STARTED);
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

static void AddCheckpoints (Emergence::Celerity::PipelineBuilder &_pipelineBuilder)
{
    _pipelineBuilder.AddCheckpoint (Checkpoint::STARTED);
    _pipelineBuilder.AddCheckpoint (Checkpoint::FINISHED);
}

void AddToFixedUpdate (Emergence::Celerity::PipelineBuilder &_pipelineBuilder) noexcept
{
    AddCheckpoints (_pipelineBuilder);
    _pipelineBuilder.AddTask (TaskNames::PROCESS_LIFETIME).SetExecutor<LifetimeProcessor> ();
    _pipelineBuilder.AddTask (TaskNames::PROCESS_DAMAGE).SetExecutor<DamageProcessor> ();
    _pipelineBuilder.AddTask (TaskNames::PROCESS_CORPSES).SetExecutor<CorpseProcessor> ();

    _pipelineBuilder.AddTask (Emergence::Memory::UniqueString {"Mortality::RemoveMortals"})
        .AS_CASCADE_REMOVER_1F (Emergence::Celerity::Transform3dComponentRemovedFixedEvent, MortalComponent, objectId)
        .DependOn (TaskNames::PROCESS_CORPSES)
        // Because mortality is de facto last mechanics in the graph, we're taking care or hierarchy cleanup here.
        .DependOn (Emergence::Celerity::TransformHierarchyCleanup::Checkpoint::DETACHED_REMOVAL_FINISHED)
        .MakeDependencyOf (Checkpoint::FINISHED)
        .MakeDependencyOf (Emergence::Celerity::TransformHierarchyCleanup::Checkpoint::DETACHMENT_DETECTION_STARTED);
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
    _constructor.DependOn (Checkpoint::STARTED);
    _constructor.DependOn (Emergence::Celerity::Assembly::Checkpoint::FINISHED);
    _constructor.MakeDependencyOf (Checkpoint::FINISHED);
    _constructor.MakeDependencyOf (Urho3DUpdate::Checkpoint::STARTED);
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
    AddCheckpoints (_pipelineBuilder);
    _pipelineBuilder.AddTask (TaskNames::TRIGGER_DEATH_EFFECT).SetExecutor<DeathEffectTrigger> ();
}
} // namespace Mortality

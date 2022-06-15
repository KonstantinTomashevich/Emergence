#include <Celerity/PipelineBuilderMacros.hpp>

#include <Gameplay/Damage.hpp>
#include <Gameplay/DamageDealerComponent.hpp>
#include <Gameplay/Events.hpp>

#include <Physics/Events.hpp>
#include <Physics/Simulation.hpp>
#include <Shared/Checkpoint.hpp>

#include <Transform/Events.hpp>
#include <Transform/Transform3dComponent.hpp>

namespace Damage
{
namespace TaskNames
{
static const Emergence::Memory::UniqueString APPLY_DAMAGE_FROM_COLLISION ("Damage::ApplyFromCollision");
static const Emergence::Memory::UniqueString CLEANUP_UNITS_AFTER_TRANSFORM_REMOVAL (
    "Damage::CleanupAfterTransformRemoval");
} // namespace TaskNames

class CollisionEventProcessor : public Emergence::Celerity::TaskExecutorBase<CollisionEventProcessor>
{
public:
    CollisionEventProcessor (Emergence::Celerity::TaskConstructor &_constructor) noexcept;

    void Execute () noexcept;

private:
    void ProcessCollision (Emergence::Celerity::UniqueId _possibleDamageDealerId,
                           Emergence::Celerity::UniqueId _victimId) noexcept;

    Emergence::Celerity::FetchSequenceQuery fetchContactFoundEvents;
    Emergence::Celerity::FetchSequenceQuery fetchTriggerEnteredEvents;

    Emergence::Celerity::EditValueQuery editDamageDealerById;
    Emergence::Celerity::RemoveValueQuery removeTransformById;

    Emergence::Celerity::InsertShortTermQuery insertDamageEvent;
};

CollisionEventProcessor::CollisionEventProcessor (Emergence::Celerity::TaskConstructor &_constructor) noexcept
    : fetchContactFoundEvents (_constructor.MFetchSequence (Emergence::Physics::ContactFoundEvent)),
      fetchTriggerEnteredEvents (_constructor.MFetchSequence (Emergence::Physics::TriggerEnteredEvent)),

      editDamageDealerById (_constructor.MEditValue1F (DamageDealerComponent, objectId)),
      removeTransformById (_constructor.MRemoveValue1F (Emergence::Transform::Transform3dComponent, objectId)),

      insertDamageEvent (_constructor.MInsertShortTerm (DamageEvent))
{
    _constructor.DependOn (Emergence::Physics::Simulation::Checkpoint::SIMULATION_FINISHED);
    _constructor.DependOn (Checkpoint::DAMAGE_STARTED);
    _constructor.MakeDependencyOf (Checkpoint::DAMAGE_FINISHED);
}

void CollisionEventProcessor::Execute () noexcept
{
    for (auto eventCursor = fetchContactFoundEvents.Execute ();
         const auto *event = static_cast<const Emergence::Physics::ContactFoundEvent *> (*eventCursor); ++eventCursor)
    {
        if (event->initialContact)
        {
            ProcessCollision (event->firstObjectId, event->secondObjectId);
            ProcessCollision (event->secondObjectId, event->firstObjectId);
        }
    }

    for (auto eventCursor = fetchTriggerEnteredEvents.Execute ();
         const auto *event = static_cast<const Emergence::Physics::TriggerEnteredEvent *> (*eventCursor); ++eventCursor)
    {
        ProcessCollision (event->triggerObjectId, event->intruderObjectId);
        ProcessCollision (event->intruderObjectId, event->triggerObjectId);
    }
}

void CollisionEventProcessor::ProcessCollision (Emergence::Celerity::UniqueId _possibleDamageDealerId,
                                                Emergence::Celerity::UniqueId _victimId) noexcept
{
    auto damageDealerCursor = editDamageDealerById.Execute (&_possibleDamageDealerId);
    auto *damageDealer = static_cast<DamageDealerComponent *> (*damageDealerCursor);

    if (!damageDealer || !damageDealer->enabled)
    {
        return;
    }

    auto eventCursor = insertDamageEvent.Execute ();
    auto *damageEvent = static_cast<DamageEvent *> (++eventCursor);

    damageEvent->objectId = _victimId;
    damageEvent->amount = damageDealer->damage;

    if (!damageDealer->multiUse)
    {
        damageDealer->enabled = false;
        auto transformCursor = removeTransformById.Execute (&_possibleDamageDealerId);

        if (transformCursor.ReadConst ())
        {
            ~transformCursor;
        }
    }
}

class TransformEventProcessor : public Emergence::Celerity::TaskExecutorBase<TransformEventProcessor>
{
public:
    TransformEventProcessor (Emergence::Celerity::TaskConstructor &_constructor) noexcept;

    void Execute () noexcept;

private:
    Emergence::Celerity::FetchSequenceQuery fetchTransformRemovedEvents;
    Emergence::Celerity::RemoveValueQuery removeDamageDealerById;
};

TransformEventProcessor::TransformEventProcessor (Emergence::Celerity::TaskConstructor &_constructor) noexcept
    : fetchTransformRemovedEvents (
          _constructor.MFetchSequence (Emergence::Transform::Transform3dComponentRemovedFixedEvent)),
      removeDamageDealerById (_constructor.MRemoveValue1F (DamageDealerComponent, objectId))

{
    // Deletion is done after mortality to avoid unneeded graph complications due to event processing.
    _constructor.DependOn (Checkpoint::MORTALITY_FINISHED);
}

void TransformEventProcessor::Execute () noexcept
{
    for (auto eventCursor = fetchTransformRemovedEvents.Execute ();
         const auto *event =
             static_cast<const Emergence::Transform::Transform3dComponentRemovedFixedEvent *> (*eventCursor);
         ++eventCursor)
    {
        auto damageDealerCursor = removeDamageDealerById.Execute (&event->objectId);
        if (damageDealerCursor.ReadConst ())
        {
            ~damageDealerCursor;
        }
    }
}

void AddToFixedUpdate (Emergence::Celerity::PipelineBuilder &_pipelineBuilder) noexcept
{
    _pipelineBuilder.AddTask (TaskNames::CLEANUP_UNITS_AFTER_TRANSFORM_REMOVAL).SetExecutor<TransformEventProcessor> ();
    _pipelineBuilder.AddTask (TaskNames::APPLY_DAMAGE_FROM_COLLISION).SetExecutor<CollisionEventProcessor> ();
}
} // namespace Damage

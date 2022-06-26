#include <Celerity/Physics/Events.hpp>
#include <Celerity/Physics/Simulation.hpp>
#include <Celerity/PipelineBuilderMacros.hpp>
#include <Celerity/Transform/Events.hpp>
#include <Celerity/Transform/Transform3dComponent.hpp>

#include <Gameplay/Damage.hpp>
#include <Gameplay/DamageDealerComponent.hpp>
#include <Gameplay/Events.hpp>

#include <Shared/Checkpoint.hpp>

namespace Damage
{
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
    : fetchContactFoundEvents (FETCH_SEQUENCE (Emergence::Celerity::ContactFoundEvent)),
      fetchTriggerEnteredEvents (FETCH_SEQUENCE (Emergence::Celerity::TriggerEnteredEvent)),

      editDamageDealerById (EDIT_VALUE_1F (DamageDealerComponent, objectId)),
      removeTransformById (REMOVE_VALUE_1F (Emergence::Celerity::Transform3dComponent, objectId)),

      insertDamageEvent (INSERT_SHORT_TERM (DamageEvent))
{
    _constructor.DependOn (Emergence::Celerity::Simulation::Checkpoint::SIMULATION_FINISHED);
    _constructor.DependOn (Checkpoint::DAMAGE_STARTED);
    _constructor.MakeDependencyOf (Checkpoint::DAMAGE_FINISHED);
}

void CollisionEventProcessor::Execute () noexcept
{
    for (auto eventCursor = fetchContactFoundEvents.Execute ();
         const auto *event = static_cast<const Emergence::Celerity::ContactFoundEvent *> (*eventCursor); ++eventCursor)
    {
        if (event->initialContact)
        {
            ProcessCollision (event->firstObjectId, event->secondObjectId);
            ProcessCollision (event->secondObjectId, event->firstObjectId);
        }
    }

    for (auto eventCursor = fetchTriggerEnteredEvents.Execute ();
         const auto *event = static_cast<const Emergence::Celerity::TriggerEnteredEvent *> (*eventCursor);
         ++eventCursor)
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

void AddToFixedUpdate (Emergence::Celerity::PipelineBuilder &_pipelineBuilder) noexcept
{
    using namespace Emergence::Memory::Literals;

    _pipelineBuilder.AddTask ("Damage::RemoveDamageDealers"_us)
        .AS_CASCADE_REMOVER_1F (Emergence::Celerity::Transform3dComponentRemovedFixedEvent, DamageDealerComponent,
                                objectId)
        // Deletion is done after mortality to avoid unneeded graph complications due to event processing.
        .DependOn (Checkpoint::MORTALITY_FINISHED);

    _pipelineBuilder.AddTask ("Damage::ApplyFromCollision"_us).SetExecutor<CollisionEventProcessor> ();
}
} // namespace Damage

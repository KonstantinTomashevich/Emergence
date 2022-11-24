#include <Celerity/Physics2d/CollisionShape2dComponent.hpp>
#include <Celerity/Physics2d/DynamicsMaterial2d.hpp>
#include <Celerity/Physics2d/Events.hpp>
#include <Celerity/Physics2d/PhysicsWorld2dSingleton.hpp>
#include <Celerity/Physics2d/RigidBody2dComponent.hpp>

namespace Emergence::Celerity
{
EMERGENCE_CELERITY_EVENT0_IMPLEMENTATION (PhysicsWorld2dConfigurationChanged)

EMERGENCE_CELERITY_EVENT1_IMPLEMENTATION (DynamicsMaterial2dAddedFixedEvent, id)

EMERGENCE_CELERITY_EVENT1_IMPLEMENTATION (DynamicsMaterial2dAddedCustomToFixedEvent, id)

EMERGENCE_CELERITY_EVENT1_IMPLEMENTATION (DynamicsMaterial2dChangedEvent, id)

EMERGENCE_CELERITY_EVENT1_IMPLEMENTATION (DynamicsMaterial2dRemovedEvent, id)

EMERGENCE_CELERITY_EVENT1_IMPLEMENTATION (CollisionShape2dComponentAddedFixedEvent, shapeId)

EMERGENCE_CELERITY_EVENT1_IMPLEMENTATION (CollisionShape2dComponentAddedCustomToFixedEvent, shapeId)

EMERGENCE_CELERITY_EVENT1_IMPLEMENTATION (CollisionShape2dComponentMaterialChangedEvent, shapeId)

EMERGENCE_CELERITY_EVENT1_IMPLEMENTATION (CollisionShape2dComponentGeometryChangedEvent, shapeId)

EMERGENCE_CELERITY_EVENT1_IMPLEMENTATION (CollisionShape2dComponentAttributesChangedEvent, shapeId)

EMERGENCE_CELERITY_EVENT2_IMPLEMENTATION (CollisionShape2dComponentRemovedEvent, objectId, implementationHandle)

EMERGENCE_CELERITY_EVENT1_IMPLEMENTATION (RigidBody2dComponentAddedFixedEvent, objectId)

EMERGENCE_CELERITY_EVENT1_IMPLEMENTATION (RigidBody2dComponentAddedCustomToFixedEvent, objectId)

EMERGENCE_CELERITY_EVENT2_IMPLEMENTATION (RigidBody2dComponentRemovedEvent, objectId, implementationHandle)

EMERGENCE_CELERITY_EVENT5_IMPLEMENTATION (
    Contact2dFoundEvent, firstObjectId, firstShapeId, secondObjectId, secondShapeId, initialContact)

EMERGENCE_CELERITY_EVENT4_IMPLEMENTATION (
    Contact2dPersistsEvent, firstObjectId, firstShapeId, secondObjectId, secondShapeId)

EMERGENCE_CELERITY_EVENT5_IMPLEMENTATION (
    Contact2dLostEvent, firstObjectId, firstShapeId, secondObjectId, secondShapeId, lastContact)

EMERGENCE_CELERITY_EVENT4_IMPLEMENTATION (
    Trigger2dEnteredEvent, triggerObjectId, triggerShapeId, intruderObjectId, intruderShapeId)

EMERGENCE_CELERITY_EVENT4_IMPLEMENTATION (
    Trigger2dExitedEvent, triggerObjectId, triggerShapeId, intruderObjectId, intruderShapeId)

void RegisterPhysicsEvents (Celerity::EventRegistrar &_registrar) noexcept
{
    // PhysicsWorld2dSingleton

    Container::Vector<StandardLayout::FieldId> physicsWorldTrackedFields {PhysicsWorld2dSingleton::Reflect ().gravity};
    for (StandardLayout::FieldId collisionMask : PhysicsWorld2dSingleton::Reflect ().collisionMasks)
    {
        physicsWorldTrackedFields.emplace_back (collisionMask);
    }

    _registrar.OnChangeEvent ({{PhysicsWorld2dConfigurationChanged::Reflect ().mapping, Celerity::EventRoute::FIXED},
                               PhysicsWorld2dSingleton::Reflect ().mapping,
                               physicsWorldTrackedFields,
                               {},
                               {}});

    // DynamicsMaterial

    _registrar.OnAddEvent ({{DynamicsMaterial2dAddedFixedEvent::Reflect ().mapping, Celerity::EventRoute::FIXED},
                            DynamicsMaterial2d::Reflect ().mapping,
                            {{DynamicsMaterial2d::Reflect ().id, DynamicsMaterial2dAddedFixedEvent::Reflect ().id}}});

    _registrar.OnAddEvent (
        {{DynamicsMaterial2dAddedCustomToFixedEvent::Reflect ().mapping, Celerity::EventRoute::FROM_CUSTOM_TO_FIXED},
         DynamicsMaterial2d::Reflect ().mapping,
         {{DynamicsMaterial2d::Reflect ().id, DynamicsMaterial2dAddedCustomToFixedEvent::Reflect ().id}}});

    _registrar.OnChangeEvent (
        {{DynamicsMaterial2dChangedEvent::Reflect ().mapping, Celerity::EventRoute::FIXED},
         DynamicsMaterial2d::Reflect ().mapping,
         {
             DynamicsMaterial2d::Reflect ().friction,
             DynamicsMaterial2d::Reflect ().restitution,
             DynamicsMaterial2d::Reflect ().restitutionThreshold,
             DynamicsMaterial2d::Reflect ().density,
         },
         {},
         {{DynamicsMaterial2d::Reflect ().id, DynamicsMaterial2dAddedFixedEvent::Reflect ().id}}});

    _registrar.OnRemoveEvent ({{DynamicsMaterial2dRemovedEvent::Reflect ().mapping, Celerity::EventRoute::FIXED},
                               DynamicsMaterial2d::Reflect ().mapping,
                               {
                                   {DynamicsMaterial2d::Reflect ().id, DynamicsMaterial2dRemovedEvent::Reflect ().id},
                               }});

    // CollisionShape2dComponent

    _registrar.OnAddEvent ({{CollisionShape2dComponentAddedFixedEvent::Reflect ().mapping, Celerity::EventRoute::FIXED},
                            CollisionShape2dComponent::Reflect ().mapping,
                            {{CollisionShape2dComponent::Reflect ().shapeId,
                              CollisionShape2dComponentAddedFixedEvent::Reflect ().shapeId}}});

    _registrar.OnAddEvent ({{CollisionShape2dComponentAddedCustomToFixedEvent::Reflect ().mapping,
                             Celerity::EventRoute::FROM_CUSTOM_TO_FIXED},
                            CollisionShape2dComponent::Reflect ().mapping,
                            {{CollisionShape2dComponent::Reflect ().shapeId,
                              CollisionShape2dComponentAddedCustomToFixedEvent::Reflect ().shapeId}}});

    _registrar.OnChangeEvent (
        {{CollisionShape2dComponentMaterialChangedEvent::Reflect ().mapping, Celerity::EventRoute::FIXED},
         CollisionShape2dComponent::Reflect ().mapping,
         {CollisionShape2dComponent::Reflect ().materialId},
         {},
         {{CollisionShape2dComponent::Reflect ().shapeId,
           CollisionShape2dComponentMaterialChangedEvent::Reflect ().shapeId}}});

    _registrar.OnChangeEvent (
        {{CollisionShape2dComponentGeometryChangedEvent::Reflect ().mapping, Celerity::EventRoute::FIXED},
         CollisionShape2dComponent::Reflect ().mapping,
         {
             CollisionShape2dComponent::Reflect ().rotation,
             CollisionShape2dComponent::Reflect ().translation,
             CollisionShape2dComponent::Reflect ().geometry,
         },
         {},
         {{CollisionShape2dComponent::Reflect ().shapeId,
           CollisionShape2dComponentGeometryChangedEvent::Reflect ().shapeId}}});

    _registrar.OnChangeEvent (
        {{CollisionShape2dComponentAttributesChangedEvent::Reflect ().mapping, Celerity::EventRoute::FIXED},
         CollisionShape2dComponent::Reflect ().mapping,
         {

             CollisionShape2dComponent::Reflect ().enabled,
             CollisionShape2dComponent::Reflect ().trigger,
             CollisionShape2dComponent::Reflect ().visibleToWorldQueries,
             CollisionShape2dComponent::Reflect ().sendContactEvents,
             CollisionShape2dComponent::Reflect ().collisionGroup,
         },
         {},
         {{CollisionShape2dComponent::Reflect ().shapeId,
           CollisionShape2dComponentAttributesChangedEvent::Reflect ().shapeId}}});

    _registrar.OnRemoveEvent ({{CollisionShape2dComponentRemovedEvent::Reflect ().mapping, Celerity::EventRoute::FIXED},
                               CollisionShape2dComponent::Reflect ().mapping,
                               {
                                   {CollisionShape2dComponent::Reflect ().objectId,
                                    CollisionShape2dComponentRemovedEvent::Reflect ().objectId},
                                   {CollisionShape2dComponent::Reflect ().implementationHandle,
                                    CollisionShape2dComponentRemovedEvent::Reflect ().implementationHandle},
                               }});

    // RigidBody2dComponent

    _registrar.OnAddEvent (
        {{RigidBody2dComponentAddedFixedEvent::Reflect ().mapping, Celerity::EventRoute::FIXED},
         RigidBody2dComponent::Reflect ().mapping,
         {{RigidBody2dComponent::Reflect ().objectId, RigidBody2dComponentAddedFixedEvent::Reflect ().objectId}}});

    _registrar.OnAddEvent (
        {{RigidBody2dComponentAddedCustomToFixedEvent::Reflect ().mapping, Celerity::EventRoute::FROM_CUSTOM_TO_FIXED},
         RigidBody2dComponent::Reflect ().mapping,
         {{RigidBody2dComponent::Reflect ().objectId,
           RigidBody2dComponentAddedCustomToFixedEvent::Reflect ().objectId}}});

    _registrar.OnRemoveEvent (
        {{RigidBody2dComponentRemovedEvent::Reflect ().mapping, Celerity::EventRoute::FIXED},
         RigidBody2dComponent::Reflect ().mapping,
         {
             {RigidBody2dComponent::Reflect ().objectId, RigidBody2dComponentRemovedEvent::Reflect ().objectId},
             {RigidBody2dComponent::Reflect ().implementationHandle,
              RigidBody2dComponentRemovedEvent::Reflect ().implementationHandle},
         }});

    // Simulation.

    _registrar.CustomEvent ({Contact2dFoundEvent::Reflect ().mapping, Celerity::EventRoute::FIXED});
    _registrar.CustomEvent ({Contact2dPersistsEvent::Reflect ().mapping, Celerity::EventRoute::FIXED});
    _registrar.CustomEvent ({Contact2dLostEvent::Reflect ().mapping, Celerity::EventRoute::FIXED});
    _registrar.CustomEvent ({Trigger2dEnteredEvent::Reflect ().mapping, Celerity::EventRoute::FIXED});
    _registrar.CustomEvent ({Trigger2dExitedEvent::Reflect ().mapping, Celerity::EventRoute::FIXED});
}
} // namespace Emergence::Celerity

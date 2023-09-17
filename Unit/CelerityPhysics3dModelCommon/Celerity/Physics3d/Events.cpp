#include <Celerity/Physics3d/CollisionShape3dComponent.hpp>
#include <Celerity/Physics3d/DynamicsMaterial3d.hpp>
#include <Celerity/Physics3d/Events.hpp>
#include <Celerity/Physics3d/PhysicsWorld3dSingleton.hpp>
#include <Celerity/Physics3d/RigidBody3dComponent.hpp>

namespace Emergence::Celerity
{
EMERGENCE_CELERITY_EVENT0_IMPLEMENTATION (PhysicsWorld3dConfigurationChanged)

EMERGENCE_CELERITY_EVENT1_IMPLEMENTATION (DynamicsMaterial3dAddedFixedEvent, id)

EMERGENCE_CELERITY_EVENT1_IMPLEMENTATION (DynamicsMaterial3dAddedCustomToFixedEvent, id)

EMERGENCE_CELERITY_EVENT1_IMPLEMENTATION (DynamicsMaterial3dChangedEvent, id)

EMERGENCE_CELERITY_EVENT2_IMPLEMENTATION (DynamicsMaterial3dRemovedEvent, id, implementationHandle)

EMERGENCE_CELERITY_EVENT1_IMPLEMENTATION (CollisionShape3dComponentAddedFixedEvent, shapeId)

EMERGENCE_CELERITY_EVENT1_IMPLEMENTATION (CollisionShape3dComponentAddedCustomToFixedEvent, shapeId)

EMERGENCE_CELERITY_EVENT1_IMPLEMENTATION (CollisionShape3dComponentMaterialChangedEvent, shapeId)

EMERGENCE_CELERITY_EVENT1_IMPLEMENTATION (CollisionShape3dComponentGeometryChangedEvent, shapeId)

EMERGENCE_CELERITY_EVENT1_IMPLEMENTATION (CollisionShape3dComponentAttributesChangedEvent, shapeId)

EMERGENCE_CELERITY_EVENT2_IMPLEMENTATION (CollisionShape3dComponentRemovedEvent, objectId, implementationHandle)

EMERGENCE_CELERITY_EVENT1_IMPLEMENTATION (RigidBody3dComponentAddedFixedEvent, objectId)

EMERGENCE_CELERITY_EVENT1_IMPLEMENTATION (RigidBody3dComponentAddedCustomToFixedEvent, objectId)

EMERGENCE_CELERITY_EVENT1_IMPLEMENTATION (RigidBody3dComponentMassInvalidatedEvent, objectId);

EMERGENCE_CELERITY_EVENT2_IMPLEMENTATION (RigidBody3dComponentRemovedEvent, objectId, implementationHandle)

EMERGENCE_CELERITY_EVENT5_IMPLEMENTATION (
    Contact3dFoundEvent, firstObjectId, firstShapeId, secondObjectId, secondShapeId, initialContact)

EMERGENCE_CELERITY_EVENT4_IMPLEMENTATION (
    Contact3dPersistsEvent, firstObjectId, firstShapeId, secondObjectId, secondShapeId)

EMERGENCE_CELERITY_EVENT5_IMPLEMENTATION (
    Contact3dLostEvent, firstObjectId, firstShapeId, secondObjectId, secondShapeId, lastContact)

EMERGENCE_CELERITY_EVENT4_IMPLEMENTATION (
    Trigger3dEnteredEvent, triggerObjectId, triggerShapeId, intruderObjectId, intruderShapeId)

EMERGENCE_CELERITY_EVENT4_IMPLEMENTATION (
    Trigger3dExitedEvent, triggerObjectId, triggerShapeId, intruderObjectId, intruderShapeId)

void RegisterPhysicsEvents (EventRegistrar &_registrar) noexcept
{
    // PhysicsWorld3dSingleton

    Container::Vector<StandardLayout::FieldId> physicsWorldTrackedFields {PhysicsWorld3dSingleton::Reflect ().gravity};
    for (StandardLayout::FieldId collisionMask : PhysicsWorld3dSingleton::Reflect ().collisionMasks)
    {
        physicsWorldTrackedFields.emplace_back (collisionMask);
    }

    _registrar.OnChangeEvent ({{PhysicsWorld3dConfigurationChanged::Reflect ().mapping, EventRoute::FIXED},
                               PhysicsWorld3dSingleton::Reflect ().mapping,
                               physicsWorldTrackedFields,
                               {},
                               {}});

    // DynamicsMaterial

    _registrar.OnAddEvent ({{DynamicsMaterial3dAddedFixedEvent::Reflect ().mapping, EventRoute::FIXED},
                            DynamicsMaterial3d::Reflect ().mapping,
                            {{DynamicsMaterial3d::Reflect ().id, DynamicsMaterial3dAddedFixedEvent::Reflect ().id}}});

    _registrar.OnAddEvent (
        {{DynamicsMaterial3dAddedCustomToFixedEvent::Reflect ().mapping, EventRoute::FROM_CUSTOM_TO_FIXED},
         DynamicsMaterial3d::Reflect ().mapping,
         {{DynamicsMaterial3d::Reflect ().id, DynamicsMaterial3dAddedCustomToFixedEvent::Reflect ().id}}});

    _registrar.OnChangeEvent (
        {{DynamicsMaterial3dChangedEvent::Reflect ().mapping, EventRoute::FIXED},
         DynamicsMaterial3d::Reflect ().mapping,
         {
             DynamicsMaterial3d::Reflect ().dynamicFriction,
             DynamicsMaterial3d::Reflect ().staticFriction,
             DynamicsMaterial3d::Reflect ().enableFriction,
             DynamicsMaterial3d::Reflect ().restitution,
             DynamicsMaterial3d::Reflect ().density,
         },
         {},
         {{DynamicsMaterial3d::Reflect ().id, DynamicsMaterial3dAddedFixedEvent::Reflect ().id}}});

    _registrar.OnRemoveEvent ({{DynamicsMaterial3dRemovedEvent::Reflect ().mapping, EventRoute::FIXED},
                               DynamicsMaterial3d::Reflect ().mapping,
                               {
                                   {DynamicsMaterial3d::Reflect ().id, DynamicsMaterial3dRemovedEvent::Reflect ().id},
                                   {DynamicsMaterial3d::Reflect ().implementationHandle,
                                    DynamicsMaterial3dRemovedEvent::Reflect ().implementationHandle},
                               }});

    // CollisionShape3dComponent

    _registrar.OnAddEvent ({{CollisionShape3dComponentAddedFixedEvent::Reflect ().mapping, EventRoute::FIXED},
                            CollisionShape3dComponent::Reflect ().mapping,
                            {{CollisionShape3dComponent::Reflect ().shapeId,
                              CollisionShape3dComponentAddedFixedEvent::Reflect ().shapeId}}});

    _registrar.OnAddEvent (
        {{CollisionShape3dComponentAddedCustomToFixedEvent::Reflect ().mapping, EventRoute::FROM_CUSTOM_TO_FIXED},
         CollisionShape3dComponent::Reflect ().mapping,
         {{CollisionShape3dComponent::Reflect ().shapeId,
           CollisionShape3dComponentAddedCustomToFixedEvent::Reflect ().shapeId}}});

    _registrar.OnChangeEvent ({{CollisionShape3dComponentMaterialChangedEvent::Reflect ().mapping, EventRoute::FIXED},
                               CollisionShape3dComponent::Reflect ().mapping,
                               {CollisionShape3dComponent::Reflect ().materialId},
                               {},
                               {{CollisionShape3dComponent::Reflect ().shapeId,
                                 CollisionShape3dComponentMaterialChangedEvent::Reflect ().shapeId}}});

    _registrar.OnChangeEvent ({{CollisionShape3dComponentGeometryChangedEvent::Reflect ().mapping, EventRoute::FIXED},
                               CollisionShape3dComponent::Reflect ().mapping,
                               {
                                   CollisionShape3dComponent::Reflect ().rotation,
                                   CollisionShape3dComponent::Reflect ().translation,
                                   CollisionShape3dComponent::Reflect ().geometry,
                               },
                               {},
                               {{CollisionShape3dComponent::Reflect ().shapeId,
                                 CollisionShape3dComponentGeometryChangedEvent::Reflect ().shapeId}}});

    _registrar.OnChangeEvent ({{CollisionShape3dComponentAttributesChangedEvent::Reflect ().mapping, EventRoute::FIXED},
                               CollisionShape3dComponent::Reflect ().mapping,
                               {

                                   CollisionShape3dComponent::Reflect ().enabled,
                                   CollisionShape3dComponent::Reflect ().trigger,
                                   CollisionShape3dComponent::Reflect ().visibleToWorldQueries,
                                   CollisionShape3dComponent::Reflect ().sendContactEvents,
                                   CollisionShape3dComponent::Reflect ().collisionGroup,
                               },
                               {},
                               {{CollisionShape3dComponent::Reflect ().shapeId,
                                 CollisionShape3dComponentAttributesChangedEvent::Reflect ().shapeId}}});

    _registrar.OnRemoveEvent ({{CollisionShape3dComponentRemovedEvent::Reflect ().mapping, EventRoute::FIXED},
                               CollisionShape3dComponent::Reflect ().mapping,
                               {
                                   {CollisionShape3dComponent::Reflect ().objectId,
                                    CollisionShape3dComponentRemovedEvent::Reflect ().objectId},
                                   {CollisionShape3dComponent::Reflect ().implementationHandle,
                                    CollisionShape3dComponentRemovedEvent::Reflect ().implementationHandle},
                               }});

    // RigidBody3dComponent

    _registrar.OnAddEvent (
        {{RigidBody3dComponentAddedFixedEvent::Reflect ().mapping, EventRoute::FIXED},
         RigidBody3dComponent::Reflect ().mapping,
         {{RigidBody3dComponent::Reflect ().objectId, RigidBody3dComponentAddedFixedEvent::Reflect ().objectId}}});

    _registrar.OnAddEvent (
        {{RigidBody3dComponentAddedCustomToFixedEvent::Reflect ().mapping, EventRoute::FROM_CUSTOM_TO_FIXED},
         RigidBody3dComponent::Reflect ().mapping,
         {{RigidBody3dComponent::Reflect ().objectId,
           RigidBody3dComponentAddedCustomToFixedEvent::Reflect ().objectId}}});

    _registrar.CustomEvent ({RigidBody3dComponentMassInvalidatedEvent::Reflect ().mapping, EventRoute::FIXED});

    _registrar.OnRemoveEvent (
        {{RigidBody3dComponentRemovedEvent::Reflect ().mapping, EventRoute::FIXED},
         RigidBody3dComponent::Reflect ().mapping,
         {
             {RigidBody3dComponent::Reflect ().objectId, RigidBody3dComponentRemovedEvent::Reflect ().objectId},
             {RigidBody3dComponent::Reflect ().implementationHandle,
              RigidBody3dComponentRemovedEvent::Reflect ().implementationHandle},
         }});

    // Simulation.

    _registrar.CustomEvent ({Contact3dFoundEvent::Reflect ().mapping, EventRoute::FIXED});
    _registrar.CustomEvent ({Contact3dPersistsEvent::Reflect ().mapping, EventRoute::FIXED});
    _registrar.CustomEvent ({Contact3dLostEvent::Reflect ().mapping, EventRoute::FIXED});
    _registrar.CustomEvent ({Trigger3dEnteredEvent::Reflect ().mapping, EventRoute::FIXED});
    _registrar.CustomEvent ({Trigger3dExitedEvent::Reflect ().mapping, EventRoute::FIXED});
}
} // namespace Emergence::Celerity

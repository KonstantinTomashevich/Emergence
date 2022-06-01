#include <Physics/CollisionShapeComponent.hpp>
#include <Physics/DynamicsMaterial.hpp>
#include <Physics/Events.hpp>
#include <Physics/PhysicsWorldSingleton.hpp>
#include <Physics/RigidBodyComponent.hpp>

namespace Emergence::Physics
{
EMERGENCE_CELERITY_EVENT0_IMPLEMENTATION (PhysicsWorldConfigurationChanged)

EMERGENCE_CELERITY_EVENT1_IMPLEMENTATION (DynamicsMaterialAddedEvent, UNIQUE_STRING, id)

EMERGENCE_CELERITY_EVENT1_IMPLEMENTATION (DynamicsMaterialChangedEvent, UNIQUE_STRING, id)

EMERGENCE_CELERITY_EVENT2_IMPLEMENTATION (
    DynamicsMaterialRemovedEvent, UNIQUE_STRING, id, POINTER_AS_REGULAR, implementationHandle)

EMERGENCE_CELERITY_EVENT1_IMPLEMENTATION (CollisionShapeComponentAddedEvent, REGULAR, shapeId)

EMERGENCE_CELERITY_EVENT1_IMPLEMENTATION (CollisionShapeComponentMaterialChangedEvent, REGULAR, shapeId)

EMERGENCE_CELERITY_EVENT1_IMPLEMENTATION (CollisionShapeComponentGeometryChangedEvent, REGULAR, shapeId)

EMERGENCE_CELERITY_EVENT1_IMPLEMENTATION (CollisionShapeComponentAttributesChangedEvent, REGULAR, shapeId)

EMERGENCE_CELERITY_EVENT2_IMPLEMENTATION (
    CollisionShapeComponentRemovedEvent, REGULAR, objectId, POINTER_AS_REGULAR, implementationHandle)

EMERGENCE_CELERITY_EVENT1_IMPLEMENTATION (RigidBodyComponentAddedEvent, REGULAR, objectId)

EMERGENCE_CELERITY_EVENT1_IMPLEMENTATION (RigidBodyComponentMassInvalidatedEvent, REGULAR, objectId);

EMERGENCE_CELERITY_EVENT2_IMPLEMENTATION (
    RigidBodyComponentRemovedEvent, REGULAR, objectId, POINTER_AS_REGULAR, implementationHandle)

EMERGENCE_CELERITY_EVENT5_IMPLEMENTATION (ContactFoundEvent,
                                          REGULAR,
                                          firstObjectId,
                                          REGULAR,
                                          firstShapeId,
                                          REGULAR,
                                          secondObjectId,
                                          REGULAR,
                                          secondShapeId,
                                          REGULAR,
                                          initialContact)

EMERGENCE_CELERITY_EVENT4_IMPLEMENTATION (ContactPersistsEvent,
                                          REGULAR,
                                          firstObjectId,
                                          REGULAR,
                                          firstShapeId,
                                          REGULAR,
                                          secondObjectId,
                                          REGULAR,
                                          secondShapeId)

EMERGENCE_CELERITY_EVENT5_IMPLEMENTATION (ContactLostEvent,
                                          REGULAR,
                                          firstObjectId,
                                          REGULAR,
                                          firstShapeId,
                                          REGULAR,
                                          secondObjectId,
                                          REGULAR,
                                          secondShapeId,
                                          REGULAR,
                                          lastContact)

EMERGENCE_CELERITY_EVENT4_IMPLEMENTATION (TriggerEnteredEvent,
                                          REGULAR,
                                          triggerObjectId,
                                          REGULAR,
                                          triggerShapeId,
                                          REGULAR,
                                          intruderObjectId,
                                          REGULAR,
                                          intruderShapeId)

EMERGENCE_CELERITY_EVENT4_IMPLEMENTATION (TriggerExitedEvent,
                                          REGULAR,
                                          triggerObjectId,
                                          REGULAR,
                                          triggerShapeId,
                                          REGULAR,
                                          intruderObjectId,
                                          REGULAR,
                                          intruderShapeId)

void RegisterEvents (Celerity::EventRegistrar &_registrar) noexcept
{
    // PhysicsWorldSingleton

    _registrar.OnChangeEvent ({{PhysicsWorldConfigurationChanged::Reflect ().mapping, Celerity::EventRoute::FIXED},
                               PhysicsWorldSingleton::Reflect ().mapping,
                               {
                                   PhysicsWorldSingleton::Reflect ().gravity,
                                   PhysicsWorldSingleton::Reflect ().collisionMaskBlock,
                               },
                               {},
                               {}});

    // DynamicsMaterial

    _registrar.OnAddEvent ({{DynamicsMaterialAddedEvent::Reflect ().mapping, Celerity::EventRoute::FIXED},
                            DynamicsMaterial::Reflect ().mapping,
                            {{DynamicsMaterial::Reflect ().id, DynamicsMaterialAddedEvent::Reflect ().id}}});

    _registrar.OnChangeEvent ({{DynamicsMaterialChangedEvent::Reflect ().mapping, Celerity::EventRoute::FIXED},
                               DynamicsMaterial::Reflect ().mapping,
                               {
                                   DynamicsMaterial::Reflect ().dynamicFriction,
                                   DynamicsMaterial::Reflect ().staticFriction,
                                   DynamicsMaterial::Reflect ().enableFriction,
                                   DynamicsMaterial::Reflect ().restitution,
                                   DynamicsMaterial::Reflect ().density,
                               },
                               {},
                               {{DynamicsMaterial::Reflect ().id, DynamicsMaterialAddedEvent::Reflect ().id}}});

    _registrar.OnRemoveEvent ({{DynamicsMaterialRemovedEvent::Reflect ().mapping, Celerity::EventRoute::FIXED},
                               DynamicsMaterial::Reflect ().mapping,
                               {
                                   {DynamicsMaterial::Reflect ().id, DynamicsMaterialRemovedEvent::Reflect ().id},
                                   {DynamicsMaterial::Reflect ().implementationHandle,
                                    DynamicsMaterialRemovedEvent::Reflect ().implementationHandle},
                               }});

    // CollisionShapeComponent

    _registrar.OnAddEvent (
        {{CollisionShapeComponentAddedEvent::Reflect ().mapping, Celerity::EventRoute::FIXED},
         CollisionShapeComponent::Reflect ().mapping,
         {{CollisionShapeComponent::Reflect ().shapeId, CollisionShapeComponentAddedEvent::Reflect ().shapeId}}});

    _registrar.OnChangeEvent (
        {{CollisionShapeComponentMaterialChangedEvent::Reflect ().mapping, Celerity::EventRoute::FIXED},
         CollisionShapeComponent::Reflect ().mapping,
         {CollisionShapeComponent::Reflect ().materialId},
         {},
         {{CollisionShapeComponent::Reflect ().shapeId,
           CollisionShapeComponentMaterialChangedEvent::Reflect ().shapeId}}});

    _registrar.OnChangeEvent (
        {{CollisionShapeComponentGeometryChangedEvent::Reflect ().mapping, Celerity::EventRoute::FIXED},
         CollisionShapeComponent::Reflect ().mapping,
         {
             CollisionShapeComponent::Reflect ().geometry,
             CollisionShapeComponent::Reflect ().translation,
             CollisionShapeComponent::Reflect ().rotation,
         },
         {},
         {{CollisionShapeComponent::Reflect ().shapeId,
           CollisionShapeComponentGeometryChangedEvent::Reflect ().shapeId}}});

    _registrar.OnChangeEvent (
        {{CollisionShapeComponentAttributesChangedEvent::Reflect ().mapping, Celerity::EventRoute::FIXED},
         CollisionShapeComponent::Reflect ().mapping,
         {

             CollisionShapeComponent::Reflect ().enabled,
             CollisionShapeComponent::Reflect ().trigger,
             CollisionShapeComponent::Reflect ().visibleToWorldQueries,
             CollisionShapeComponent::Reflect ().collisionGroup,
         },
         {},
         {{CollisionShapeComponent::Reflect ().shapeId,
           CollisionShapeComponentAttributesChangedEvent::Reflect ().shapeId}}});

    _registrar.OnRemoveEvent (
        {{CollisionShapeComponentRemovedEvent::Reflect ().mapping, Celerity::EventRoute::FIXED},
         CollisionShapeComponent::Reflect ().mapping,
         {
             {CollisionShapeComponent::Reflect ().objectId, CollisionShapeComponentRemovedEvent::Reflect ().objectId},
             {CollisionShapeComponent::Reflect ().implementationHandle,
              CollisionShapeComponentRemovedEvent::Reflect ().implementationHandle},
         }});

    // RigidBodyComponent

    _registrar.OnAddEvent (
        {{RigidBodyComponentAddedEvent::Reflect ().mapping, Celerity::EventRoute::FIXED},
         RigidBodyComponent::Reflect ().mapping,
         {{RigidBodyComponent::Reflect ().objectId, RigidBodyComponentAddedEvent::Reflect ().objectId}}});

    _registrar.CustomEvent ({RigidBodyComponentMassInvalidatedEvent::Reflect ().mapping, Celerity::EventRoute::FIXED});

    _registrar.OnRemoveEvent (
        {{RigidBodyComponentRemovedEvent::Reflect ().mapping, Celerity::EventRoute::FIXED},
         RigidBodyComponent::Reflect ().mapping,
         {
             {RigidBodyComponent::Reflect ().objectId, RigidBodyComponentRemovedEvent::Reflect ().objectId},
             {RigidBodyComponent::Reflect ().implementationHandle,
              RigidBodyComponentRemovedEvent::Reflect ().implementationHandle},
         }});

    // Simulation.

    _registrar.CustomEvent ({ContactFoundEvent::Reflect ().mapping, Celerity::EventRoute::FIXED});
    _registrar.CustomEvent ({ContactPersistsEvent::Reflect ().mapping, Celerity::EventRoute::FIXED});
    _registrar.CustomEvent ({ContactLostEvent::Reflect ().mapping, Celerity::EventRoute::FIXED});
    _registrar.CustomEvent ({TriggerEnteredEvent::Reflect ().mapping, Celerity::EventRoute::FIXED});
    _registrar.CustomEvent ({TriggerExitedEvent::Reflect ().mapping, Celerity::EventRoute::FIXED});
}
} // namespace Emergence::Physics

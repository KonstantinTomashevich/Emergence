#include <Celerity/Physics/CollisionShapeComponent.hpp>
#include <Celerity/Physics/DynamicsMaterial.hpp>
#include <Celerity/Physics/Events.hpp>
#include <Celerity/Physics/PhysicsWorldSingleton.hpp>
#include <Celerity/Physics/RigidBodyComponent.hpp>

namespace Emergence::Celerity
{
EMERGENCE_CELERITY_EVENT0_IMPLEMENTATION (PhysicsWorldConfigurationChanged)

EMERGENCE_CELERITY_EVENT1_IMPLEMENTATION (DynamicsMaterialAddedFixedEvent, id)

EMERGENCE_CELERITY_EVENT1_IMPLEMENTATION (DynamicsMaterialAddedCustomToFixedEvent, id)

EMERGENCE_CELERITY_EVENT1_IMPLEMENTATION (DynamicsMaterialChangedEvent, id)

EMERGENCE_CELERITY_EVENT2_IMPLEMENTATION (DynamicsMaterialRemovedEvent, id, implementationHandle)

EMERGENCE_CELERITY_EVENT1_IMPLEMENTATION (CollisionShapeComponentAddedFixedEvent, shapeId)

EMERGENCE_CELERITY_EVENT1_IMPLEMENTATION (CollisionShapeComponentAddedCustomToFixedEvent, shapeId)

EMERGENCE_CELERITY_EVENT1_IMPLEMENTATION (CollisionShapeComponentMaterialChangedEvent, shapeId)

EMERGENCE_CELERITY_EVENT1_IMPLEMENTATION (CollisionShapeComponentGeometryChangedEvent, shapeId)

EMERGENCE_CELERITY_EVENT1_IMPLEMENTATION (CollisionShapeComponentAttributesChangedEvent, shapeId)

EMERGENCE_CELERITY_EVENT2_IMPLEMENTATION (CollisionShapeComponentRemovedEvent, objectId, implementationHandle)

EMERGENCE_CELERITY_EVENT1_IMPLEMENTATION (RigidBodyComponentAddedFixedEvent, objectId)

EMERGENCE_CELERITY_EVENT1_IMPLEMENTATION (RigidBodyComponentAddedCustomToFixedEvent, objectId)

EMERGENCE_CELERITY_EVENT1_IMPLEMENTATION (RigidBodyComponentMassInvalidatedEvent, objectId);

EMERGENCE_CELERITY_EVENT2_IMPLEMENTATION (RigidBodyComponentRemovedEvent, objectId, implementationHandle)

EMERGENCE_CELERITY_EVENT5_IMPLEMENTATION (
    ContactFoundEvent, firstObjectId, firstShapeId, secondObjectId, secondShapeId, initialContact)

EMERGENCE_CELERITY_EVENT4_IMPLEMENTATION (
    ContactPersistsEvent, firstObjectId, firstShapeId, secondObjectId, secondShapeId)

EMERGENCE_CELERITY_EVENT5_IMPLEMENTATION (
    ContactLostEvent, firstObjectId, firstShapeId, secondObjectId, secondShapeId, lastContact)

EMERGENCE_CELERITY_EVENT4_IMPLEMENTATION (
    TriggerEnteredEvent, triggerObjectId, triggerShapeId, intruderObjectId, intruderShapeId)

EMERGENCE_CELERITY_EVENT4_IMPLEMENTATION (
    TriggerExitedEvent, triggerObjectId, triggerShapeId, intruderObjectId, intruderShapeId)

void RegisterPhysicsEvents (Celerity::EventRegistrar &_registrar) noexcept
{
    // PhysicsWorldSingleton

    Container::Vector<StandardLayout::FieldId> physicsWorldTrackedFields {PhysicsWorldSingleton::Reflect ().gravity};
    for (StandardLayout::FieldId collisionMask : PhysicsWorldSingleton::Reflect ().collisionMasks)
    {
        physicsWorldTrackedFields.emplace_back (collisionMask);
    }

    _registrar.OnChangeEvent ({{PhysicsWorldConfigurationChanged::Reflect ().mapping, Celerity::EventRoute::FIXED},
                               PhysicsWorldSingleton::Reflect ().mapping,
                               physicsWorldTrackedFields,
                               {},
                               {}});

    // DynamicsMaterial

    _registrar.OnAddEvent ({{DynamicsMaterialAddedFixedEvent::Reflect ().mapping, Celerity::EventRoute::FIXED},
                            DynamicsMaterial::Reflect ().mapping,
                            {{DynamicsMaterial::Reflect ().id, DynamicsMaterialAddedFixedEvent::Reflect ().id}}});

    _registrar.OnAddEvent (
        {{DynamicsMaterialAddedCustomToFixedEvent::Reflect ().mapping, Celerity::EventRoute::FROM_CUSTOM_TO_FIXED},
         DynamicsMaterial::Reflect ().mapping,
         {{DynamicsMaterial::Reflect ().id, DynamicsMaterialAddedCustomToFixedEvent::Reflect ().id}}});

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
                               {{DynamicsMaterial::Reflect ().id, DynamicsMaterialAddedFixedEvent::Reflect ().id}}});

    _registrar.OnRemoveEvent ({{DynamicsMaterialRemovedEvent::Reflect ().mapping, Celerity::EventRoute::FIXED},
                               DynamicsMaterial::Reflect ().mapping,
                               {
                                   {DynamicsMaterial::Reflect ().id, DynamicsMaterialRemovedEvent::Reflect ().id},
                                   {DynamicsMaterial::Reflect ().implementationHandle,
                                    DynamicsMaterialRemovedEvent::Reflect ().implementationHandle},
                               }});

    // CollisionShapeComponent

    _registrar.OnAddEvent (
        {{CollisionShapeComponentAddedFixedEvent::Reflect ().mapping, Celerity::EventRoute::FIXED},
         CollisionShapeComponent::Reflect ().mapping,
         {{CollisionShapeComponent::Reflect ().shapeId, CollisionShapeComponentAddedFixedEvent::Reflect ().shapeId}}});

    _registrar.OnAddEvent ({{CollisionShapeComponentAddedCustomToFixedEvent::Reflect ().mapping,
                             Celerity::EventRoute::FROM_CUSTOM_TO_FIXED},
                            CollisionShapeComponent::Reflect ().mapping,
                            {{CollisionShapeComponent::Reflect ().shapeId,
                              CollisionShapeComponentAddedCustomToFixedEvent::Reflect ().shapeId}}});

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
             CollisionShapeComponent::Reflect ().rotation,
             CollisionShapeComponent::Reflect ().translation,
             CollisionShapeComponent::Reflect ().geometry,
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
             CollisionShapeComponent::Reflect ().sendContactEvents,
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
        {{RigidBodyComponentAddedFixedEvent::Reflect ().mapping, Celerity::EventRoute::FIXED},
         RigidBodyComponent::Reflect ().mapping,
         {{RigidBodyComponent::Reflect ().objectId, RigidBodyComponentAddedFixedEvent::Reflect ().objectId}}});

    _registrar.OnAddEvent (
        {{RigidBodyComponentAddedCustomToFixedEvent::Reflect ().mapping, Celerity::EventRoute::FROM_CUSTOM_TO_FIXED},
         RigidBodyComponent::Reflect ().mapping,
         {{RigidBodyComponent::Reflect ().objectId, RigidBodyComponentAddedCustomToFixedEvent::Reflect ().objectId}}});

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
} // namespace Emergence::Celerity

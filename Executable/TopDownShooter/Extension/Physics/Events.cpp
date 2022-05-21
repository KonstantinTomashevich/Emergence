#include <Physics/CollisionShapeComponent.hpp>
#include <Physics/DynamicsMaterial.hpp>
#include <Physics/Events.hpp>
#include <Physics/PhysicsWorldSingleton.hpp>

namespace Emergence::Physics
{
EMERGENCE_CELERITY_EVENT1_IMPLEMENTATION (DynamicsMaterialAddedEvent, UNIQUE_STRING, id)

EMERGENCE_CELERITY_EVENT1_IMPLEMENTATION (DynamicsMaterialChangedEvent, UNIQUE_STRING, id)

EMERGENCE_CELERITY_EVENT1_IMPLEMENTATION (DynamicsMaterialRemovedEvent, UNIQUE_STRING, id)

EMERGENCE_CELERITY_EVENT1_IMPLEMENTATION (CollisionShapeComponentAddedEvent, REGULAR, shapeId)

EMERGENCE_CELERITY_EVENT1_IMPLEMENTATION (CollisionShapeComponentMaterialChangedEvent, REGULAR, shapeId);

EMERGENCE_CELERITY_EVENT1_IMPLEMENTATION (CollisionShapeComponentGeometryChangedEvent, REGULAR, shapeId);

EMERGENCE_CELERITY_EVENT1_IMPLEMENTATION (CollisionShapeComponentAttributesChangedEvent, REGULAR, shapeId);

EMERGENCE_CELERITY_EVENT2_IMPLEMENTATION (
    CollisionShapeComponentRemovedEvent, REGULAR, objectId, POINTER_AS_REGULAR, implementationHandle);

void RegisterEvents (Celerity::EventRegistrar &_registrar) noexcept
{
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
                               {{DynamicsMaterial::Reflect ().id, DynamicsMaterialRemovedEvent::Reflect ().id}}});

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
}
} // namespace Emergence::Physics

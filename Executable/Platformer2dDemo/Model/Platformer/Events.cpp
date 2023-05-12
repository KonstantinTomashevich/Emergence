#include <Platformer/Events.hpp>
#include <Platformer/Spawn/SpawnMarkerComponent.hpp>

EMERGENCE_CELERITY_EVENT2_IMPLEMENTATION (ControllableComponentAddedFixedEvent, objectId, control)
EMERGENCE_CELERITY_EVENT3_IMPLEMENTATION (ControllableComponentChangedFixedEvent, objectId, oldControl, newControl)
EMERGENCE_CELERITY_EVENT2_IMPLEMENTATION (ControllableComponentRemovedFixedEvent, objectId, control)

EMERGENCE_CELERITY_EVENT1_IMPLEMENTATION (SpawnMarkerComponentRemovedFixedEvent, spawnObjectId)

void RegisterPlatformerEvents (Emergence::Celerity::EventRegistrar &_registrar) noexcept
{
    // ControllableComponent

    _registrar.OnAddEvent (
        {{ControllableComponentAddedFixedEvent::Reflect ().mapping, Emergence::Celerity::EventRoute::FIXED},
         ControllableComponent::Reflect ().mapping,
         {{ControllableComponent::Reflect ().objectId, ControllableComponentAddedFixedEvent::Reflect ().objectId},
          {ControllableComponent::Reflect ().control, ControllableComponentAddedFixedEvent::Reflect ().control}}});

    _registrar.OnChangeEvent (
        {{ControllableComponentChangedFixedEvent::Reflect ().mapping, Emergence::Celerity::EventRoute::FIXED},
         ControllableComponent::Reflect ().mapping,
         {ControllableComponent::Reflect ().control},
         {{ControllableComponent::Reflect ().control, ControllableComponentChangedFixedEvent::Reflect ().oldControl}},
         {{ControllableComponent::Reflect ().objectId, ControllableComponentChangedFixedEvent::Reflect ().objectId},
          {ControllableComponent::Reflect ().control, ControllableComponentChangedFixedEvent::Reflect ().newControl}}});

    _registrar.OnRemoveEvent (
        {{ControllableComponentRemovedFixedEvent::Reflect ().mapping, Emergence::Celerity::EventRoute::FIXED},
         ControllableComponent::Reflect ().mapping,
         {{ControllableComponent::Reflect ().objectId, ControllableComponentRemovedFixedEvent::Reflect ().objectId},
          {ControllableComponent::Reflect ().control, ControllableComponentRemovedFixedEvent::Reflect ().control}}});

    // SpawnMarkerComponent

    _registrar.OnRemoveEvent (
        {{SpawnMarkerComponentRemovedFixedEvent::Reflect ().mapping, Emergence::Celerity::EventRoute::FIXED},
         SpawnMarkerComponent::Reflect ().mapping,
         {{SpawnMarkerComponent::Reflect ().spawnObjectId,
           SpawnMarkerComponentRemovedFixedEvent::Reflect ().spawnObjectId}}});
}

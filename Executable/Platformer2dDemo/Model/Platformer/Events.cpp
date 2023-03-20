#include <Platformer/Events.hpp>
#include <Platformer/Spawn/SpawnMarkerComponent.hpp>

EMERGENCE_CELERITY_EVENT1_IMPLEMENTATION (SpawnMarkerComponentRemovedFixedEvent, spawnObjectId)

void RegisterPlatformerEvents (Emergence::Celerity::EventRegistrar &_registrar) noexcept
{
    // SpawnMarkerComponent

    _registrar.OnRemoveEvent (
        {{SpawnMarkerComponentRemovedFixedEvent::Reflect ().mapping, Emergence::Celerity::EventRoute::FIXED},
         SpawnMarkerComponent::Reflect ().mapping,
         {{SpawnMarkerComponent::Reflect ().spawnObjectId,
           SpawnMarkerComponentRemovedFixedEvent::Reflect ().spawnObjectId}}});
}

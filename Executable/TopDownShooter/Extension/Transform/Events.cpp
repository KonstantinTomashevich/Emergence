#include <Transform/Events.hpp>
#include <Transform/Transform3dComponent.hpp>

namespace Emergence::Transform
{
EMERGENCE_CELERITY_EVENT1_IMPLEMENTATION (Transform3dComponentRemovedFixedEvent, REGULAR, objectId);

void RegisterEvents (Celerity::EventRegistrar &_registrar) noexcept
{
    _registrar.OnRemoveEvent (
        {{Transform3dComponentRemovedFixedEvent::Reflect ().mapping, Celerity::EventRoute::FIXED},
         Transform3dComponent::Reflect ().mapping,
         {{Transform3dComponent::Reflect ().objectId, Transform3dComponentRemovedFixedEvent::Reflect ().objectId}}});
}
} // namespace Emergence::Transform

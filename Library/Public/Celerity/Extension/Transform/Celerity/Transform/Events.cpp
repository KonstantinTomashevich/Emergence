#include <Celerity/Transform/Events.hpp>
#include <Celerity/Transform/Transform3dComponent.hpp>

namespace Emergence::Celerity
{
EMERGENCE_CELERITY_EVENT1_IMPLEMENTATION (Transform3dComponentRemovedFixedEvent, objectId);
EMERGENCE_CELERITY_EVENT1_IMPLEMENTATION (Transform3dComponentRemovedNormalEvent, objectId);
EMERGENCE_CELERITY_EVENT1_IMPLEMENTATION (Transform3dComponentRemovedFixedToNormalEvent, objectId);

void RegisterTransformEvents (EventRegistrar &_registrar) noexcept
{
    _registrar.OnRemoveEvent (
        {{Transform3dComponentRemovedFixedEvent::Reflect ().mapping, EventRoute::FIXED},
         Transform3dComponent::Reflect ().mapping,
         {{Transform3dComponent::Reflect ().objectId, Transform3dComponentRemovedFixedEvent::Reflect ().objectId}}});

    _registrar.OnRemoveEvent (
        {{Transform3dComponentRemovedNormalEvent::Reflect ().mapping, EventRoute::NORMAL},
         Transform3dComponent::Reflect ().mapping,
         {{Transform3dComponent::Reflect ().objectId, Transform3dComponentRemovedNormalEvent::Reflect ().objectId}}});

    _registrar.OnRemoveEvent (
        {{Transform3dComponentRemovedFixedToNormalEvent::Reflect ().mapping, EventRoute::FROM_FIXED_TO_NORMAL},
         Transform3dComponent::Reflect ().mapping,
         {{Transform3dComponent::Reflect ().objectId,
           Transform3dComponentRemovedFixedToNormalEvent::Reflect ().objectId}}});
}
} // namespace Emergence::Celerity

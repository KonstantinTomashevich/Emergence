#include <Transform/Events.hpp>
#include <Transform/Transform3dComponent.hpp>

namespace Emergence::Transform
{
EMERGENCE_CELERITY_EVENT1_IMPLEMENTATION (Transform3dComponentRemovedFixedEvent, REGULAR, objectId);
EMERGENCE_CELERITY_EVENT1_IMPLEMENTATION (Transform3dComponentRemovedNormalEvent, REGULAR, objectId);
EMERGENCE_CELERITY_EVENT1_IMPLEMENTATION (Transform3dComponentRemovedFixedToNormalEvent, REGULAR, objectId);

void RegisterEvents (Celerity::EventRegistrar &_registrar) noexcept
{
    _registrar.OnRemoveEvent (
        {{Transform3dComponentRemovedFixedEvent::Reflect ().mapping, Celerity::EventRoute::FIXED},
         Transform3dComponent::Reflect ().mapping,
         {{Transform3dComponent::Reflect ().objectId, Transform3dComponentRemovedFixedEvent::Reflect ().objectId}}});

    _registrar.OnRemoveEvent (
        {{Transform3dComponentRemovedNormalEvent::Reflect ().mapping, Celerity::EventRoute::NORMAL},
         Transform3dComponent::Reflect ().mapping,
         {{Transform3dComponent::Reflect ().objectId, Transform3dComponentRemovedNormalEvent::Reflect ().objectId}}});

    _registrar.OnRemoveEvent ({{Transform3dComponentRemovedFixedToNormalEvent::Reflect ().mapping,
                                Celerity::EventRoute::FROM_FIXED_TO_NORMAL},
                               Transform3dComponent::Reflect ().mapping,
                               {{Transform3dComponent::Reflect ().objectId,
                                 Transform3dComponentRemovedFixedToNormalEvent::Reflect ().objectId}}});
}
} // namespace Emergence::Transform

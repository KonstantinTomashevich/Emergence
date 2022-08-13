#include <Celerity/Transform/Events.hpp>
#include <Celerity/Transform/Transform3dComponent.hpp>

namespace Emergence::Celerity
{
EMERGENCE_CELERITY_EVENT1_IMPLEMENTATION (Transform3dComponentAddedFixedToNormalEvent, objectId);
EMERGENCE_CELERITY_EVENT1_IMPLEMENTATION (Transform3dComponentAddedNormalEvent, objectId);

EMERGENCE_CELERITY_EVENT1_IMPLEMENTATION (Transform3dComponentVisualLocalTransformChangedFixedToNormalEvent, objectId);
EMERGENCE_CELERITY_EVENT1_IMPLEMENTATION (Transform3dComponentVisualLocalTransformChangedNormalEvent, objectId);

EMERGENCE_CELERITY_EVENT1_IMPLEMENTATION (Transform3dComponentRemovedFixedEvent, objectId);
EMERGENCE_CELERITY_EVENT1_IMPLEMENTATION (Transform3dComponentRemovedNormalEvent, objectId);
EMERGENCE_CELERITY_EVENT1_IMPLEMENTATION (Transform3dComponentRemovedFixedToNormalEvent, objectId);

void RegisterTransformEvents (EventRegistrar &_registrar) noexcept
{
    _registrar.OnAddEvent (
        {{Transform3dComponentAddedFixedToNormalEvent::Reflect ().mapping, EventRoute::FROM_FIXED_TO_NORMAL},
         Transform3dComponent::Reflect ().mapping,
         {{Transform3dComponent::Reflect ().objectId,
           Transform3dComponentAddedFixedToNormalEvent::Reflect ().objectId}}});

    _registrar.OnAddEvent (
        {{Transform3dComponentAddedNormalEvent::Reflect ().mapping, EventRoute::NORMAL},
         Transform3dComponent::Reflect ().mapping,
         {{Transform3dComponent::Reflect ().objectId, Transform3dComponentAddedNormalEvent::Reflect ().objectId}}});

    _registrar.OnChangeEvent (
        {{Transform3dComponentVisualLocalTransformChangedNormalEvent::Reflect ().mapping, EventRoute::NORMAL},
         Transform3dComponent::Reflect ().mapping,
         {Transform3dComponent::Reflect ().visualLocalTransform},
         {},
         {{Transform3dComponent::Reflect ().objectId,
           Transform3dComponentVisualLocalTransformChangedNormalEvent::Reflect ().objectId}}});

    _registrar.OnChangeEvent (
        {{Transform3dComponentVisualLocalTransformChangedFixedToNormalEvent::Reflect ().mapping,
          EventRoute::FROM_FIXED_TO_NORMAL},
         Transform3dComponent::Reflect ().mapping,
         {Transform3dComponent::Reflect ().visualLocalTransform},
         {},
         {{Transform3dComponent::Reflect ().objectId,
           Transform3dComponentVisualLocalTransformChangedFixedToNormalEvent::Reflect ().objectId}}});

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

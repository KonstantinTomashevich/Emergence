#include <Celerity/Transform/Events.hpp>
#include <Celerity/Transform/TransformComponent.hpp>

namespace Emergence::Celerity
{
#define IMPLEMENT_TRANSFORM_EVENTS(Dimension)                                                                          \
    EMERGENCE_CELERITY_EVENT1_IMPLEMENTATION (Transform##Dimension##dComponentAddedFixedToNormalEvent, objectId)       \
    EMERGENCE_CELERITY_EVENT1_IMPLEMENTATION (Transform##Dimension##dComponentAddedNormalEvent, objectId)              \
                                                                                                                       \
    EMERGENCE_CELERITY_EVENT1_IMPLEMENTATION (                                                                         \
        Transform##Dimension##dComponentVisualLocalTransformChangedFixedToNormalEvent, objectId);                      \
    EMERGENCE_CELERITY_EVENT1_IMPLEMENTATION (Transform##Dimension##dComponentVisualLocalTransformChangedNormalEvent,  \
                                              objectId)                                                                \
                                                                                                                       \
    EMERGENCE_CELERITY_EVENT2_IMPLEMENTATION (Transform##Dimension##dComponentParentChangedFixedToNormalEvent,         \
                                              objectId, oldParentId)                                                   \
    EMERGENCE_CELERITY_EVENT2_IMPLEMENTATION (Transform##Dimension##dComponentParentChangedNormalEvent, objectId,      \
                                              oldParentId)                                                             \
                                                                                                                       \
    EMERGENCE_CELERITY_EVENT1_IMPLEMENTATION (Transform##Dimension##dComponentRemovedFixedEvent, objectId)             \
    EMERGENCE_CELERITY_EVENT1_IMPLEMENTATION (Transform##Dimension##dComponentRemovedNormalEvent, objectId)            \
    EMERGENCE_CELERITY_EVENT1_IMPLEMENTATION (Transform##Dimension##dComponentRemovedFixedToNormalEvent, objectId)

IMPLEMENT_TRANSFORM_EVENTS (2)
IMPLEMENT_TRANSFORM_EVENTS (3)

EMERGENCE_CELERITY_EVENT1_IMPLEMENTATION (TransformNodeCleanupFixedEvent, objectId)
EMERGENCE_CELERITY_EVENT2_IMPLEMENTATION (TransformNodeCleanupNormalEvent, objectId, cleanupTransform)

#define REGISTER_TRANSFORM_EVENTS(Dimension)                                                                           \
    _registrar.OnAddEvent ({{Transform##Dimension##dComponentAddedFixedToNormalEvent::Reflect ().mapping,              \
                             EventRoute::FROM_FIXED_TO_NORMAL},                                                        \
                            Transform##Dimension##dComponent::Reflect ().mapping,                                      \
                            {{Transform##Dimension##dComponent::Reflect ().objectId,                                   \
                              Transform##Dimension##dComponentAddedFixedToNormalEvent::Reflect ().objectId}}});        \
                                                                                                                       \
    _registrar.OnAddEvent (                                                                                            \
        {{Transform##Dimension##dComponentAddedNormalEvent::Reflect ().mapping, EventRoute::NORMAL},                   \
         Transform##Dimension##dComponent::Reflect ().mapping,                                                         \
         {{Transform##Dimension##dComponent::Reflect ().objectId,                                                      \
           Transform##Dimension##dComponentAddedNormalEvent::Reflect ().objectId}}});                                  \
                                                                                                                       \
    _registrar.OnChangeEvent (                                                                                         \
        {{Transform##Dimension##dComponentVisualLocalTransformChangedNormalEvent::Reflect ().mapping,                  \
          EventRoute::NORMAL},                                                                                         \
         Transform##Dimension##dComponent::Reflect ().mapping,                                                         \
         {Transform##Dimension##dComponent::Reflect ().visualLocalTransform},                                          \
         {},                                                                                                           \
         {{Transform##Dimension##dComponent::Reflect ().objectId,                                                      \
           Transform##Dimension##dComponentVisualLocalTransformChangedNormalEvent::Reflect ().objectId}}});            \
                                                                                                                       \
    _registrar.OnChangeEvent (                                                                                         \
        {{Transform##Dimension##dComponentVisualLocalTransformChangedFixedToNormalEvent::Reflect ().mapping,           \
          EventRoute::FROM_FIXED_TO_NORMAL},                                                                           \
         Transform##Dimension##dComponent::Reflect ().mapping,                                                         \
         {Transform##Dimension##dComponent::Reflect ().visualLocalTransform},                                          \
         {},                                                                                                           \
         {{Transform##Dimension##dComponent::Reflect ().objectId,                                                      \
           Transform##Dimension##dComponentVisualLocalTransformChangedFixedToNormalEvent::Reflect ().objectId}}});     \
                                                                                                                       \
    _registrar.OnChangeEvent (                                                                                         \
        {{Transform##Dimension##dComponentParentChangedNormalEvent::Reflect ().mapping, EventRoute::NORMAL},           \
         Transform##Dimension##dComponent::Reflect ().mapping,                                                         \
         {Transform##Dimension##dComponent::Reflect ().parentObjectId},                                                \
         {{Transform##Dimension##dComponent::Reflect ().parentObjectId,                                                \
           Transform##Dimension##dComponentParentChangedNormalEvent::Reflect ().oldParentId}},                         \
         {{Transform##Dimension##dComponent::Reflect ().objectId,                                                      \
           Transform##Dimension##dComponentParentChangedNormalEvent::Reflect ().objectId}}});                          \
                                                                                                                       \
    _registrar.OnChangeEvent (                                                                                         \
        {{Transform##Dimension##dComponentParentChangedFixedToNormalEvent::Reflect ().mapping,                         \
          EventRoute::FROM_FIXED_TO_NORMAL},                                                                           \
         Transform##Dimension##dComponent::Reflect ().mapping,                                                         \
         {Transform##Dimension##dComponent::Reflect ().parentObjectId},                                                \
         {{Transform##Dimension##dComponent::Reflect ().parentObjectId,                                                \
           Transform##Dimension##dComponentParentChangedFixedToNormalEvent::Reflect ().oldParentId}},                  \
         {{Transform##Dimension##dComponent::Reflect ().objectId,                                                      \
           Transform##Dimension##dComponentParentChangedFixedToNormalEvent::Reflect ().objectId}}});                   \
                                                                                                                       \
    _registrar.OnRemoveEvent (                                                                                         \
        {{Transform##Dimension##dComponentRemovedFixedEvent::Reflect ().mapping, EventRoute::FIXED},                   \
         Transform##Dimension##dComponent::Reflect ().mapping,                                                         \
         {{Transform##Dimension##dComponent::Reflect ().objectId,                                                      \
           Transform##Dimension##dComponentRemovedFixedEvent::Reflect ().objectId}}});                                 \
                                                                                                                       \
    _registrar.OnRemoveEvent (                                                                                         \
        {{Transform##Dimension##dComponentRemovedNormalEvent::Reflect ().mapping, EventRoute::NORMAL},                 \
         Transform##Dimension##dComponent::Reflect ().mapping,                                                         \
         {{Transform##Dimension##dComponent::Reflect ().objectId,                                                      \
           Transform##Dimension##dComponentRemovedNormalEvent::Reflect ().objectId}}});                                \
                                                                                                                       \
    _registrar.OnRemoveEvent ({{Transform##Dimension##dComponentRemovedFixedToNormalEvent::Reflect ().mapping,         \
                                EventRoute::FROM_FIXED_TO_NORMAL},                                                     \
                               Transform##Dimension##dComponent::Reflect ().mapping,                                   \
                               {{Transform##Dimension##dComponent::Reflect ().objectId,                                \
                                 Transform##Dimension##dComponentRemovedFixedToNormalEvent::Reflect ().objectId}}})

void RegisterTransform2dEvents (EventRegistrar &_registrar) noexcept
{
    REGISTER_TRANSFORM_EVENTS (2);
}

void RegisterTransform3dEvents (EventRegistrar &_registrar) noexcept
{
    REGISTER_TRANSFORM_EVENTS (3);
}

void RegisterTransformCommonEvents (EventRegistrar &_registrar) noexcept
{
    _registrar.CustomEvent ({TransformNodeCleanupFixedEvent::Reflect ().mapping, EventRoute::FIXED});
    _registrar.CustomEvent ({TransformNodeCleanupNormalEvent::Reflect ().mapping, EventRoute::NORMAL});
}
} // namespace Emergence::Celerity

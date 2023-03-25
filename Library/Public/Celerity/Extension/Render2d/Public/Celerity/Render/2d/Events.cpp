#include <Celerity/Render/2d/DebugShape2dComponent.hpp>
#include <Celerity/Render/2d/Events.hpp>
#include <Celerity/Render/2d/Sprite2dComponent.hpp>

namespace Emergence::Celerity
{
EMERGENCE_CELERITY_EVENT2_IMPLEMENTATION (Sprite2dAddedNormalEvent, objectId, spriteId)
EMERGENCE_CELERITY_EVENT2_IMPLEMENTATION (Sprite2dSizeChangedNormalEvent, objectId, spriteId)
EMERGENCE_CELERITY_EVENT1_IMPLEMENTATION (Sprite2dRemovedNormalEvent, objectId)

EMERGENCE_CELERITY_EVENT2_IMPLEMENTATION (DebugShape2dAddedNormalEvent, objectId, debugShapeId)
EMERGENCE_CELERITY_EVENT2_IMPLEMENTATION (DebugShape2dAddedFixedToNormalEvent, objectId, debugShapeId)
EMERGENCE_CELERITY_EVENT2_IMPLEMENTATION (DebugShape2dGeometryChangedNormalEvent, objectId, debugShapeId)
EMERGENCE_CELERITY_EVENT2_IMPLEMENTATION (DebugShape2dGeometryChangedFixedToNormalEvent, objectId, debugShapeId)
EMERGENCE_CELERITY_EVENT1_IMPLEMENTATION (DebugShape2dRemovedNormalEvent, objectId)
EMERGENCE_CELERITY_EVENT1_IMPLEMENTATION (DebugShape2dRemovedFixedToNormalEvent, objectId)

void RegisterRender2dEvents (EventRegistrar &_registrar) noexcept
{
    // Sprite2dComponent

    _registrar.OnAddEvent ({{Sprite2dAddedNormalEvent::Reflect ().mapping, EventRoute::NORMAL},
                            Sprite2dComponent::Reflect ().mapping,
                            {{Sprite2dComponent::Reflect ().objectId, Sprite2dAddedNormalEvent::Reflect ().objectId},
                             {Sprite2dComponent::Reflect ().spriteId, Sprite2dAddedNormalEvent::Reflect ().spriteId}}});

    _registrar.OnChangeEvent (
        {{Sprite2dSizeChangedNormalEvent::Reflect ().mapping, EventRoute::NORMAL},
         Sprite2dComponent::Reflect ().mapping,
         {Sprite2dComponent::Reflect ().halfSize},
         {},
         {{Sprite2dComponent::Reflect ().objectId, Sprite2dRemovedNormalEvent::Reflect ().objectId},
          {Sprite2dComponent::Reflect ().spriteId, Sprite2dSizeChangedNormalEvent::Reflect ().spriteId}}});

    _registrar.OnRemoveEvent (
        {{Sprite2dRemovedNormalEvent::Reflect ().mapping, EventRoute::NORMAL},
         Sprite2dComponent::Reflect ().mapping,
         {{Sprite2dComponent::Reflect ().objectId, Sprite2dRemovedNormalEvent::Reflect ().objectId}}});

    // DebugShape2dComponent

    _registrar.OnAddEvent (
        {{DebugShape2dAddedNormalEvent::Reflect ().mapping, EventRoute::NORMAL},
         DebugShape2dComponent::Reflect ().mapping,
         {{DebugShape2dComponent::Reflect ().objectId, DebugShape2dAddedNormalEvent::Reflect ().objectId},
          {DebugShape2dComponent::Reflect ().debugShapeId, DebugShape2dAddedNormalEvent::Reflect ().debugShapeId}}});

    _registrar.OnAddEvent (
        {{DebugShape2dAddedFixedToNormalEvent::Reflect ().mapping, EventRoute::FROM_FIXED_TO_NORMAL},
         DebugShape2dComponent::Reflect ().mapping,
         {{DebugShape2dComponent::Reflect ().objectId, DebugShape2dAddedFixedToNormalEvent::Reflect ().objectId},
          {DebugShape2dComponent::Reflect ().debugShapeId,
           DebugShape2dAddedFixedToNormalEvent::Reflect ().debugShapeId}}});

    _registrar.OnChangeEvent (
        {{DebugShape2dGeometryChangedNormalEvent::Reflect ().mapping, EventRoute::NORMAL},
         DebugShape2dComponent::Reflect ().mapping,
         {
             DebugShape2dComponent::Reflect ().translation,
             DebugShape2dComponent::Reflect ().rotation,
             DebugShape2dComponent::Reflect ().shape,
         },
         {},
         {{DebugShape2dComponent::Reflect ().objectId, DebugShape2dRemovedNormalEvent::Reflect ().objectId},
          {DebugShape2dComponent::Reflect ().debugShapeId,
           DebugShape2dGeometryChangedNormalEvent::Reflect ().debugShapeId}}});

    _registrar.OnChangeEvent (
        {{DebugShape2dGeometryChangedFixedToNormalEvent::Reflect ().mapping, EventRoute::FROM_FIXED_TO_NORMAL},
         DebugShape2dComponent::Reflect ().mapping,
         {
             DebugShape2dComponent::Reflect ().translation,
             DebugShape2dComponent::Reflect ().rotation,
             DebugShape2dComponent::Reflect ().shape,
         },
         {},
         {{DebugShape2dComponent::Reflect ().objectId, DebugShape2dRemovedFixedToNormalEvent::Reflect ().objectId},
          {DebugShape2dComponent::Reflect ().debugShapeId,
           DebugShape2dGeometryChangedFixedToNormalEvent::Reflect ().debugShapeId}}});

    _registrar.OnRemoveEvent (
        {{DebugShape2dRemovedNormalEvent::Reflect ().mapping, EventRoute::NORMAL},
         DebugShape2dComponent::Reflect ().mapping,
         {{DebugShape2dComponent::Reflect ().objectId, DebugShape2dRemovedNormalEvent::Reflect ().objectId}}});

    _registrar.OnRemoveEvent (
        {{DebugShape2dRemovedFixedToNormalEvent::Reflect ().mapping, EventRoute::FROM_FIXED_TO_NORMAL},
         DebugShape2dComponent::Reflect ().mapping,
         {{DebugShape2dComponent::Reflect ().objectId, DebugShape2dRemovedFixedToNormalEvent::Reflect ().objectId}}});
}
} // namespace Emergence::Celerity

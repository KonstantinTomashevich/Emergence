#include <Celerity/Render2d/Events.hpp>
#include <Celerity/Render2d/Sprite2dComponent.hpp>

namespace Emergence::Celerity
{
EMERGENCE_CELERITY_EVENT2_IMPLEMENTATION (Sprite2dAddedNormalEvent, objectId, spriteId)
EMERGENCE_CELERITY_EVENT2_IMPLEMENTATION (Sprite2dSizeChangedNormalEvent, objectId, spriteId)
EMERGENCE_CELERITY_EVENT1_IMPLEMENTATION (Sprite2dRemovedNormalEvent, objectId)

void RegisterRender2dEvents (EventRegistrar &_registrar) noexcept
{
    _registrar.OnAddEvent ({{Sprite2dAddedNormalEvent::Reflect ().mapping, EventRoute::NORMAL},
                            Sprite2dComponent::Reflect ().mapping,
                            {{Sprite2dComponent::Reflect ().objectId, Sprite2dRemovedNormalEvent::Reflect ().objectId},
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
}
} // namespace Emergence::Celerity

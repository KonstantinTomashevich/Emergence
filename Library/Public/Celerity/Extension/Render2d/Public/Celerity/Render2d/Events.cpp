#include <Celerity/Render2d/Events.hpp>
#include <Celerity/Render2d/Sprite2dComponent.hpp>
#include <Celerity/Render2d/Viewport2d.hpp>

namespace Emergence::Celerity
{
EMERGENCE_CELERITY_EVENT2_IMPLEMENTATION (Sprite2dAddedNormalEvent, objectId, spriteId)
EMERGENCE_CELERITY_EVENT2_IMPLEMENTATION (Sprite2dSizeChangedNormalEvent, objectId, spriteId)
EMERGENCE_CELERITY_EVENT1_IMPLEMENTATION (Sprite2dRemovedNormalEvent, objectId)

EMERGENCE_CELERITY_EVENT1_IMPLEMENTATION (Viewport2dAddedNormalEvent, name);
EMERGENCE_CELERITY_EVENT1_IMPLEMENTATION (Viewport2dAddedCustomToNormalEvent, name);
EMERGENCE_CELERITY_EVENT1_IMPLEMENTATION (Viewport2dChangedNormalEvent, name);

void RegisterRender2dEvents (EventRegistrar &_registrar) noexcept
{
    // Sprite2d

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

    // Viewport2d

    _registrar.OnAddEvent ({{Viewport2dAddedNormalEvent::Reflect ().mapping, EventRoute::NORMAL},
                            Viewport2d::Reflect ().mapping,
                            {{Viewport2d::Reflect ().name, Viewport2dAddedNormalEvent::Reflect ().name}}});

    _registrar.OnAddEvent ({{Viewport2dAddedCustomToNormalEvent::Reflect ().mapping, EventRoute::FROM_CUSTOM_TO_NORMAL},
                            Viewport2d::Reflect ().mapping,
                            {{Viewport2d::Reflect ().name, Viewport2dAddedCustomToNormalEvent::Reflect ().name}}});

    _registrar.OnChangeEvent ({{Viewport2dChangedNormalEvent::Reflect ().mapping, EventRoute::NORMAL},
                               Viewport2d::Reflect ().mapping,
                               {
                                   Viewport2d::Reflect ().x,
                                   Viewport2d::Reflect ().y,
                                   Viewport2d::Reflect ().width,
                                   Viewport2d::Reflect ().height,
                                   Viewport2d::Reflect ().clearColor,
                               },
                               {},
                               {{Viewport2d::Reflect ().name, Viewport2dAddedNormalEvent::Reflect ().name}}});
}
} // namespace Emergence::Celerity

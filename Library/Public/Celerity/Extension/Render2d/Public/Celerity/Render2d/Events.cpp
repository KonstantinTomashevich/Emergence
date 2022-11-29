#include <Celerity/Render2d/Events.hpp>
#include <Celerity/Render2d/Sprite2dComponent.hpp>
#include <Celerity/Render2d/Viewport.hpp>

namespace Emergence::Celerity
{
EMERGENCE_CELERITY_EVENT2_IMPLEMENTATION (Sprite2dAddedNormalEvent, objectId, spriteId)
EMERGENCE_CELERITY_EVENT2_IMPLEMENTATION (Sprite2dSizeChangedNormalEvent, objectId, spriteId)
EMERGENCE_CELERITY_EVENT1_IMPLEMENTATION (Sprite2dRemovedNormalEvent, objectId)

EMERGENCE_CELERITY_EVENT1_IMPLEMENTATION (ViewportAddedNormalEvent, name);
EMERGENCE_CELERITY_EVENT1_IMPLEMENTATION (ViewportAddedCustomToNormalEvent, name);
EMERGENCE_CELERITY_EVENT1_IMPLEMENTATION (ViewportChangedNormalEvent, name);

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

    // Viewport

    _registrar.OnAddEvent ({{ViewportAddedNormalEvent::Reflect ().mapping, EventRoute::NORMAL},
                            Viewport::Reflect ().mapping,
                            {{Viewport::Reflect ().name, ViewportAddedNormalEvent::Reflect ().name}}});

    _registrar.OnAddEvent ({{ViewportAddedCustomToNormalEvent::Reflect ().mapping, EventRoute::FROM_CUSTOM_TO_NORMAL},
                            Viewport::Reflect ().mapping,
                            {{Viewport::Reflect ().name, ViewportAddedCustomToNormalEvent::Reflect ().name}}});

    _registrar.OnChangeEvent ({{ViewportChangedNormalEvent::Reflect ().mapping, EventRoute::NORMAL},
                               Viewport::Reflect ().mapping,
                               {
                                   Viewport::Reflect ().x,
                                   Viewport::Reflect ().y,
                                   Viewport::Reflect ().width,
                                   Viewport::Reflect ().height,
                                   Viewport::Reflect ().clearColor,
                               },
                               {},
                               {{Viewport::Reflect ().name, ViewportAddedNormalEvent::Reflect ().name}}});
}
} // namespace Emergence::Celerity

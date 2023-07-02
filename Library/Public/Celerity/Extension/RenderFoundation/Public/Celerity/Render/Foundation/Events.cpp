#include <Celerity/Render/Foundation/Events.hpp>
#include <Celerity/Render/Foundation/Viewport.hpp>

namespace Emergence::Celerity
{
EMERGENCE_CELERITY_EVENT1_IMPLEMENTATION (ViewportAddedNormalEvent, name);
EMERGENCE_CELERITY_EVENT1_IMPLEMENTATION (ViewportAddedCustomToNormalEvent, name);
EMERGENCE_CELERITY_EVENT1_IMPLEMENTATION (ViewportChangedNormalEvent, name);
EMERGENCE_CELERITY_EVENT1_IMPLEMENTATION (ViewportRemovedNormalEvent, name);

void RegisterRenderFoundationEvents (EventRegistrar &_registrar) noexcept
{
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
                                   Viewport::Reflect ().targetFrameBuffer,
                                   Viewport::Reflect ().x,
                                   Viewport::Reflect ().y,
                                   Viewport::Reflect ().width,
                                   Viewport::Reflect ().height,
                                   Viewport::Reflect ().clearColor,
                                   Viewport::Reflect ().sortMode,
                               },
                               {},
                               {{Viewport::Reflect ().name, ViewportAddedNormalEvent::Reflect ().name}}});

    _registrar.OnRemoveEvent ({{ViewportRemovedNormalEvent::Reflect ().mapping, EventRoute::NORMAL},
                               Viewport::Reflect ().mapping,
                               {{Viewport::Reflect ().name, ViewportRemovedNormalEvent::Reflect ().name}}});
}
} // namespace Emergence::Celerity

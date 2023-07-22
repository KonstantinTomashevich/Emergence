#include <Celerity/UI/Events.hpp>
#include <Celerity/UI/UINode.hpp>

namespace Emergence::Celerity
{
EMERGENCE_CELERITY_EVENT1_IMPLEMENTATION (UINodeRemovedNormalEvent, nodeId);

void RegisterUIEvents (EventRegistrar &_registrar) noexcept
{
    _registrar.OnRemoveEvent ({{UINodeRemovedNormalEvent::Reflect ().mapping, EventRoute::NORMAL},
                               UINode::Reflect ().mapping,
                               {{UINode::Reflect ().nodeId, UINodeRemovedNormalEvent::Reflect ().nodeId}}});
}
} // namespace Emergence::Celerity

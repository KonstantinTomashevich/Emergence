#include <Celerity/Assembly/Events.hpp>
#include <Celerity/Assembly/PrototypeComponent.hpp>

namespace Emergence::Celerity
{
EMERGENCE_CELERITY_EVENT1_IMPLEMENTATION (AssemblyFinishedFixedEvent, objectId);
EMERGENCE_CELERITY_EVENT1_IMPLEMENTATION (AssemblyFinishedNormalEvent, objectId);

void RegisterAssemblyEvents (EventRegistrar &_registrar) noexcept
{
    _registrar.CustomEvent ({AssemblyFinishedFixedEvent::Reflect ().mapping, EventRoute::FIXED});
    _registrar.CustomEvent ({AssemblyFinishedNormalEvent::Reflect ().mapping, EventRoute::NORMAL});
}
} // namespace Emergence::Celerity

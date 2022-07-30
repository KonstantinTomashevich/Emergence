#include <Celerity/Assembly/Events.hpp>
#include <Celerity/Assembly/PrototypeComponent.hpp>

namespace Emergence::Celerity
{
EMERGENCE_CELERITY_EVENT1_IMPLEMENTATION (PrototypeComponentAddedFixedEvent, objectId);
EMERGENCE_CELERITY_EVENT1_IMPLEMENTATION (PrototypeComponentAddedCustomToFixedEvent, objectId);

EMERGENCE_CELERITY_EVENT1_IMPLEMENTATION (PrototypeComponentAddedNormalEvent, objectId);
EMERGENCE_CELERITY_EVENT1_IMPLEMENTATION (PrototypeComponentAddedFixedToNormalEvent, objectId);
EMERGENCE_CELERITY_EVENT1_IMPLEMENTATION (PrototypeComponentAddedCustomToNormalEvent, objectId);

EMERGENCE_CELERITY_EVENT1_IMPLEMENTATION (AssemblyFinishedFixedEvent, objectId);
EMERGENCE_CELERITY_EVENT1_IMPLEMENTATION (AssemblyFinishedNormalEvent, objectId);

void RegisterAssemblyEvents (EventRegistrar &_registrar) noexcept
{
    _registrar.OnAddEvent (
        {{PrototypeComponentAddedFixedEvent::Reflect ().mapping, EventRoute::FIXED},
         PrototypeComponent::Reflect ().mapping,
         {{PrototypeComponent::Reflect ().objectId, PrototypeComponentAddedFixedEvent::Reflect ().objectId}}});

    _registrar.OnAddEvent (
        {{PrototypeComponentAddedCustomToFixedEvent::Reflect ().mapping, EventRoute::FROM_CUSTOM_TO_FIXED},
         PrototypeComponent::Reflect ().mapping,
         {{PrototypeComponent::Reflect ().objectId, PrototypeComponentAddedCustomToFixedEvent::Reflect ().objectId}}});

    _registrar.OnAddEvent (
        {{PrototypeComponentAddedNormalEvent::Reflect ().mapping, EventRoute::NORMAL},
         PrototypeComponent::Reflect ().mapping,
         {{PrototypeComponent::Reflect ().objectId, PrototypeComponentAddedNormalEvent::Reflect ().objectId}}});

    _registrar.OnAddEvent (
        {{PrototypeComponentAddedFixedToNormalEvent::Reflect ().mapping, EventRoute::FROM_FIXED_TO_NORMAL},
         PrototypeComponent::Reflect ().mapping,
         {{PrototypeComponent::Reflect ().objectId, PrototypeComponentAddedFixedToNormalEvent::Reflect ().objectId}}});

    _registrar.OnAddEvent (
        {{PrototypeComponentAddedCustomToNormalEvent::Reflect ().mapping, EventRoute::FROM_CUSTOM_TO_NORMAL},
         PrototypeComponent::Reflect ().mapping,
         {{PrototypeComponent::Reflect ().objectId, PrototypeComponentAddedCustomToNormalEvent::Reflect ().objectId}}});

    _registrar.CustomEvent ({AssemblyFinishedFixedEvent::Reflect ().mapping, EventRoute::FIXED});
    _registrar.CustomEvent ({AssemblyFinishedNormalEvent::Reflect ().mapping, EventRoute::NORMAL});
}
} // namespace Emergence::Celerity

#include <Gameplay/Events.hpp>
#include <Gameplay/PrototypeComponent.hpp>

EMERGENCE_CELERITY_EVENT2_IMPLEMENTATION (DamageEvent, REGULAR, objectId, REGULAR, amount);

EMERGENCE_CELERITY_EVENT1_IMPLEMENTATION (DeathFixedEvent, REGULAR, objectId);

EMERGENCE_CELERITY_EVENT1_IMPLEMENTATION (DeathFixedToNormalEvent, REGULAR, objectId);

EMERGENCE_CELERITY_EVENT1_IMPLEMENTATION (PrototypeComponentAddedFixedEvent, REGULAR, objectId);

EMERGENCE_CELERITY_EVENT1_IMPLEMENTATION (PrototypeComponentAddedFixedToNormalEvent, REGULAR, objectId);

EMERGENCE_CELERITY_EVENT1_IMPLEMENTATION (PrototypeComponentAddedCustomToFixedEvent, REGULAR, objectId);

EMERGENCE_CELERITY_EVENT1_IMPLEMENTATION (PrototypeComponentAddedCustomToNormalEvent, REGULAR, objectId);

EMERGENCE_CELERITY_EVENT1_IMPLEMENTATION (PrototypeAssembledFixedEvent, REGULAR, objectId);

EMERGENCE_CELERITY_EVENT1_IMPLEMENTATION (PrototypeAssembledNormalEvent, REGULAR, objectId);

void RegisterGameplayEvents (Emergence::Celerity::EventRegistrar &_registrar) noexcept
{
    _registrar.CustomEvent ({DamageEvent::Reflect ().mapping, Emergence::Celerity::EventRoute::FIXED});
    _registrar.CustomEvent ({DeathFixedEvent::Reflect ().mapping, Emergence::Celerity::EventRoute::FIXED});
    _registrar.CustomEvent (
        {DeathFixedToNormalEvent::Reflect ().mapping, Emergence::Celerity::EventRoute::FROM_FIXED_TO_NORMAL});

    // PrototypeComponent

    _registrar.OnAddEvent (
        {{PrototypeComponentAddedFixedEvent::Reflect ().mapping, Emergence::Celerity::EventRoute::FIXED},
         PrototypeComponent::Reflect ().mapping,
         {{PrototypeComponent::Reflect ().objectId, PrototypeComponentAddedFixedEvent::Reflect ().objectId}}});

    _registrar.OnAddEvent (
        {{PrototypeComponentAddedFixedToNormalEvent::Reflect ().mapping,
          Emergence::Celerity::EventRoute::FROM_FIXED_TO_NORMAL},
         PrototypeComponent::Reflect ().mapping,
         {{PrototypeComponent::Reflect ().objectId, PrototypeComponentAddedFixedToNormalEvent::Reflect ().objectId}}});

    _registrar.OnAddEvent (
        {{PrototypeComponentAddedCustomToFixedEvent::Reflect ().mapping,
          Emergence::Celerity::EventRoute::FROM_CUSTOM_TO_FIXED},
         PrototypeComponent::Reflect ().mapping,
         {{PrototypeComponent::Reflect ().objectId, PrototypeComponentAddedCustomToFixedEvent::Reflect ().objectId}}});

    _registrar.OnAddEvent (
        {{PrototypeComponentAddedCustomToNormalEvent::Reflect ().mapping,
          Emergence::Celerity::EventRoute::FROM_CUSTOM_TO_NORMAL},
         PrototypeComponent::Reflect ().mapping,
         {{PrototypeComponent::Reflect ().objectId, PrototypeComponentAddedCustomToNormalEvent::Reflect ().objectId}}});

    _registrar.CustomEvent ({PrototypeAssembledFixedEvent::Reflect ().mapping, Emergence::Celerity::EventRoute::FIXED});
    _registrar.CustomEvent (
        {PrototypeAssembledNormalEvent::Reflect ().mapping, Emergence::Celerity::EventRoute::NORMAL});
}

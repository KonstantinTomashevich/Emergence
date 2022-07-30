#include <Gameplay/Events.hpp>

EMERGENCE_CELERITY_EVENT2_IMPLEMENTATION (DamageEvent, objectId, amount);

EMERGENCE_CELERITY_EVENT1_IMPLEMENTATION (DeathFixedEvent, objectId);

EMERGENCE_CELERITY_EVENT1_IMPLEMENTATION (DeathFixedToNormalEvent, objectId);

void RegisterGameplayEvents (Emergence::Celerity::EventRegistrar &_registrar) noexcept
{
    _registrar.CustomEvent ({DamageEvent::Reflect ().mapping, Emergence::Celerity::EventRoute::FIXED});
    _registrar.CustomEvent ({DeathFixedEvent::Reflect ().mapping, Emergence::Celerity::EventRoute::FIXED});
    _registrar.CustomEvent (
        {DeathFixedToNormalEvent::Reflect ().mapping, Emergence::Celerity::EventRoute::FROM_FIXED_TO_NORMAL});
}

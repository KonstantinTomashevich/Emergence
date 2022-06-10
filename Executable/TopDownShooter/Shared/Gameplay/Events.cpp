#include <Gameplay/Events.hpp>

EMERGENCE_CELERITY_EVENT2_IMPLEMENTATION (DamageEvent, REGULAR, objectId, REGULAR, amount);

EMERGENCE_CELERITY_EVENT1_IMPLEMENTATION (DeathEvent, REGULAR, objectId);

void RegisterGameplayEvents (Emergence::Celerity::EventRegistrar &_registrar) noexcept
{
    _registrar.CustomEvent ({DamageEvent::Reflect ().mapping, Emergence::Celerity::EventRoute::FIXED});
    _registrar.CustomEvent ({DeathEvent::Reflect ().mapping, Emergence::Celerity::EventRoute::FIXED});
}

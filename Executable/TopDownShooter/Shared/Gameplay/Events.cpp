#include <Gameplay/Events.hpp>
#include <Gameplay/UnitComponent.hpp>

EMERGENCE_CELERITY_EVENT2_IMPLEMENTATION (DamageEvent, REGULAR, objectId, REGULAR, amount);

EMERGENCE_CELERITY_EVENT1_IMPLEMENTATION (DeathEvent, REGULAR, objectId);

EMERGENCE_CELERITY_EVENT1_IMPLEMENTATION (UnitComponentAddedFixedEvent, REGULAR, objectId);

EMERGENCE_CELERITY_EVENT1_IMPLEMENTATION (UnitComponentAddedFixedToNormalEvent, REGULAR, objectId);

EMERGENCE_CELERITY_EVENT1_IMPLEMENTATION (UnitComponentAddedCustomToFixedEvent, REGULAR, objectId);

EMERGENCE_CELERITY_EVENT1_IMPLEMENTATION (UnitComponentAddedCustomToNormalEvent, REGULAR, objectId);

void RegisterGameplayEvents (Emergence::Celerity::EventRegistrar &_registrar) noexcept
{
    _registrar.CustomEvent ({DamageEvent::Reflect ().mapping, Emergence::Celerity::EventRoute::FIXED});
    _registrar.CustomEvent ({DeathEvent::Reflect ().mapping, Emergence::Celerity::EventRoute::FIXED});

    // UnitComponent

    _registrar.OnAddEvent ({{UnitComponentAddedFixedEvent::Reflect ().mapping, Emergence::Celerity::EventRoute::FIXED},
                            UnitComponent::Reflect ().mapping,
                            {{UnitComponent::Reflect ().objectId, UnitComponentAddedFixedEvent::Reflect ().objectId}}});

    _registrar.OnAddEvent (
        {{UnitComponentAddedFixedToNormalEvent::Reflect ().mapping,
          Emergence::Celerity::EventRoute::FROM_FIXED_TO_NORMAL},
         UnitComponent::Reflect ().mapping,
         {{UnitComponent::Reflect ().objectId, UnitComponentAddedFixedToNormalEvent::Reflect ().objectId}}});

    _registrar.OnAddEvent (
        {{UnitComponentAddedCustomToFixedEvent::Reflect ().mapping,
          Emergence::Celerity::EventRoute::FROM_CUSTOM_TO_FIXED},
         UnitComponent::Reflect ().mapping,
         {{UnitComponent::Reflect ().objectId, UnitComponentAddedCustomToFixedEvent::Reflect ().objectId}}});

    _registrar.OnAddEvent (
        {{UnitComponentAddedCustomToNormalEvent::Reflect ().mapping,
          Emergence::Celerity::EventRoute::FROM_CUSTOM_TO_NORMAL},
         UnitComponent::Reflect ().mapping,
         {{UnitComponent::Reflect ().objectId, UnitComponentAddedCustomToNormalEvent::Reflect ().objectId}}});
}

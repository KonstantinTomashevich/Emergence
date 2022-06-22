#pragma once

#include <Celerity/Event/EventRegistrar.hpp>
#include <Celerity/Event/Macro.generated.hpp>
#include <Celerity/Standard/UniqueId.hpp>

EMERGENCE_CELERITY_EVENT2_DECLARATION (DamageEvent, Emergence::Celerity::UniqueId, objectId, float, amount);

EMERGENCE_CELERITY_EVENT1_DECLARATION (DeathEvent, Emergence::Celerity::UniqueId, objectId);

EMERGENCE_CELERITY_EVENT1_DECLARATION (PrototypeComponentAddedFixedEvent, Emergence::Celerity::UniqueId, objectId);

EMERGENCE_CELERITY_EVENT1_DECLARATION (PrototypeComponentAddedFixedToNormalEvent,
                                       Emergence::Celerity::UniqueId,
                                       objectId);

EMERGENCE_CELERITY_EVENT1_DECLARATION (PrototypeComponentAddedCustomToFixedEvent,
                                       Emergence::Celerity::UniqueId,
                                       objectId);

EMERGENCE_CELERITY_EVENT1_DECLARATION (PrototypeComponentAddedCustomToNormalEvent,
                                       Emergence::Celerity::UniqueId,
                                       objectId);

EMERGENCE_CELERITY_EVENT1_DECLARATION (PrototypeAssembledFixedEvent, Emergence::Celerity::UniqueId, objectId);

EMERGENCE_CELERITY_EVENT1_DECLARATION (PrototypeAssembledNormalEvent, Emergence::Celerity::UniqueId, objectId);

void RegisterGameplayEvents (Emergence::Celerity::EventRegistrar &_registrar) noexcept;

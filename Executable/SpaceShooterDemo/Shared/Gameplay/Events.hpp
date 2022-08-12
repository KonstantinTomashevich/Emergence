#pragma once

#include <Celerity/Event/EventRegistrar.hpp>
#include <Celerity/Event/Macro.generated.hpp>
#include <Celerity/Standard/UniqueId.hpp>

EMERGENCE_CELERITY_EVENT1_DECLARATION (MortalComponentAddedEvent, Emergence::Celerity::UniqueId, objectId);

EMERGENCE_CELERITY_EVENT2_DECLARATION (DamageEvent, Emergence::Celerity::UniqueId, objectId, float, amount);

EMERGENCE_CELERITY_EVENT1_DECLARATION (DeathFixedEvent, Emergence::Celerity::UniqueId, objectId);

EMERGENCE_CELERITY_EVENT1_DECLARATION (DeathFixedToNormalEvent, Emergence::Celerity::UniqueId, objectId);

void RegisterGameplayEvents (Emergence::Celerity::EventRegistrar &_registrar) noexcept;

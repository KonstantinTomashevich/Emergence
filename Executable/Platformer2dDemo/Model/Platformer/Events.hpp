#pragma once

#include <Celerity/Event/EventRegistrar.hpp>
#include <Celerity/Event/Macro.generated.hpp>
#include <Celerity/Standard/UniqueId.hpp>

EMERGENCE_CELERITY_EVENT1_DECLARATION (SpawnMarkerComponentRemovedFixedEvent,
                                       Emergence::Celerity::UniqueId,
                                       spawnObjectId);

void RegisterPlatformerEvents (Emergence::Celerity::EventRegistrar &_registrar) noexcept;

#pragma once

#include <Celerity/Event/EventRegistrar.hpp>
#include <Celerity/Event/Macro.generated.hpp>
#include <Celerity/Standard/UniqueId.hpp>

#include <Platformer/Control/ControllableComponent.hpp>

EMERGENCE_CELERITY_EVENT2_DECLARATION (
    ControllableComponentAddedFixedEvent, Emergence::Celerity::UniqueId, objectId, ControlType, control);
EMERGENCE_CELERITY_EVENT3_DECLARATION (ControllableComponentChangedFixedEvent,
                                       Emergence::Celerity::UniqueId,
                                       objectId,
                                       ControlType,
                                       oldControl,
                                       ControlType,
                                       newControl);
EMERGENCE_CELERITY_EVENT2_DECLARATION (
    ControllableComponentRemovedFixedEvent, Emergence::Celerity::UniqueId, objectId, ControlType, control);

EMERGENCE_CELERITY_EVENT1_DECLARATION (SpawnMarkerComponentRemovedFixedEvent,
                                       Emergence::Celerity::UniqueId,
                                       spawnObjectId);

void RegisterPlatformerEvents (Emergence::Celerity::EventRegistrar &_registrar) noexcept;

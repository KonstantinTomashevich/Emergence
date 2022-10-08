#pragma once

#include <Celerity/Event/EventRegistrar.hpp>
#include <Celerity/Event/Macro.generated.hpp>
#include <Celerity/Standard/UniqueId.hpp>

namespace Emergence::Celerity
{
#define DECLARE_TRANSFORM_EVENTS(Dimension)                                                                            \
    EMERGENCE_CELERITY_EVENT1_DECLARATION (Transform##Dimension##dComponentAddedFixedToNormalEvent, UniqueId,          \
                                           objectId);                                                                  \
    EMERGENCE_CELERITY_EVENT1_DECLARATION (Transform##Dimension##dComponentAddedNormalEvent, UniqueId, objectId);      \
                                                                                                                       \
    EMERGENCE_CELERITY_EVENT1_DECLARATION (                                                                            \
        Transform##Dimension##dComponentVisualLocalTransformChangedFixedToNormalEvent, UniqueId, objectId);            \
    EMERGENCE_CELERITY_EVENT1_DECLARATION (Transform##Dimension##dComponentVisualLocalTransformChangedNormalEvent,     \
                                           UniqueId, objectId);                                                        \
                                                                                                                       \
    EMERGENCE_CELERITY_EVENT1_DECLARATION (Transform##Dimension##dComponentRemovedFixedEvent, UniqueId, objectId);     \
    EMERGENCE_CELERITY_EVENT1_DECLARATION (Transform##Dimension##dComponentRemovedNormalEvent, UniqueId, objectId);    \
    EMERGENCE_CELERITY_EVENT1_DECLARATION (Transform##Dimension##dComponentRemovedFixedToNormalEvent, UniqueId,        \
                                           objectId);

DECLARE_TRANSFORM_EVENTS (2)
DECLARE_TRANSFORM_EVENTS (3)

void RegisterTransform2dEvents (EventRegistrar &_registrar) noexcept;

void RegisterTransform3dEvents (EventRegistrar &_registrar) noexcept;
} // namespace Emergence::Celerity

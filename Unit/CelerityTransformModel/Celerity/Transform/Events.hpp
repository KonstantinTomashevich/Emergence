#pragma once

#include <CelerityTransformModelApi.hpp>

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
    EMERGENCE_CELERITY_EVENT2_DECLARATION (Transform##Dimension##dComponentParentChangedFixedToNormalEvent, UniqueId,  \
                                           objectId, UniqueId, oldParentId);                                           \
    EMERGENCE_CELERITY_EVENT2_DECLARATION (Transform##Dimension##dComponentParentChangedNormalEvent, UniqueId,         \
                                           objectId, UniqueId, oldParentId);                                           \
                                                                                                                       \
    EMERGENCE_CELERITY_EVENT1_DECLARATION (Transform##Dimension##dComponentRemovedFixedEvent, UniqueId, objectId);     \
    EMERGENCE_CELERITY_EVENT1_DECLARATION (Transform##Dimension##dComponentRemovedNormalEvent, UniqueId, objectId);    \
    EMERGENCE_CELERITY_EVENT1_DECLARATION (Transform##Dimension##dComponentRemovedFixedToNormalEvent, UniqueId,        \
                                           objectId);

#define EventsApi CelerityTransformModelApi
DECLARE_TRANSFORM_EVENTS (2)
DECLARE_TRANSFORM_EVENTS (3)

EMERGENCE_CELERITY_EVENT1_DECLARATION (TransformNodeCleanupFixedEvent, UniqueId, objectId);
EMERGENCE_CELERITY_EVENT2_DECLARATION (TransformNodeCleanupNormalEvent, UniqueId, objectId, bool, cleanupTransform);
#undef EventsApi

CelerityTransformModelApi void RegisterTransform2dEvents (EventRegistrar &_registrar) noexcept;

CelerityTransformModelApi void RegisterTransform3dEvents (EventRegistrar &_registrar) noexcept;

CelerityTransformModelApi void RegisterTransformCommonEvents (EventRegistrar &_registrar) noexcept;
} // namespace Emergence::Celerity

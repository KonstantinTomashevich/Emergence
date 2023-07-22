#pragma once


#include <CelerityRender2dModelApi.hpp>
#include <Celerity/Event/EventRegistrar.hpp>
#include <Celerity/Event/Macro.generated.hpp>
#include <Celerity/Standard/UniqueId.hpp>

namespace Emergence::Celerity
{
#define EventsApi CelerityRender2dModelApi
EMERGENCE_CELERITY_EVENT2_DECLARATION (Sprite2dAddedNormalEvent, UniqueId, objectId, UniqueId, spriteId);
EMERGENCE_CELERITY_EVENT2_DECLARATION (Sprite2dSizeChangedNormalEvent, UniqueId, objectId, UniqueId, spriteId);
EMERGENCE_CELERITY_EVENT1_DECLARATION (Sprite2dRemovedNormalEvent, UniqueId, objectId);

EMERGENCE_CELERITY_EVENT2_DECLARATION (Sprite2dUvAnimationAddedNormalEvent, UniqueId, objectId, UniqueId, spriteId);
EMERGENCE_CELERITY_EVENT2_DECLARATION (
    Sprite2dUvAnimationSyncedValuesChangedNormalEvent, UniqueId, objectId, UniqueId, spriteId);

EMERGENCE_CELERITY_EVENT2_DECLARATION (DebugShape2dAddedNormalEvent, UniqueId, objectId, UniqueId, debugShapeId);
EMERGENCE_CELERITY_EVENT2_DECLARATION (DebugShape2dAddedFixedToNormalEvent, UniqueId, objectId, UniqueId, debugShapeId);
EMERGENCE_CELERITY_EVENT2_DECLARATION (
    DebugShape2dGeometryChangedNormalEvent, UniqueId, objectId, UniqueId, debugShapeId);
EMERGENCE_CELERITY_EVENT2_DECLARATION (
    DebugShape2dGeometryChangedFixedToNormalEvent, UniqueId, objectId, UniqueId, debugShapeId);
EMERGENCE_CELERITY_EVENT1_DECLARATION (DebugShape2dRemovedNormalEvent, UniqueId, objectId);
EMERGENCE_CELERITY_EVENT1_DECLARATION (DebugShape2dRemovedFixedToNormalEvent, UniqueId, objectId);
#undef EventsApi

CelerityRender2dModelApi void RegisterRender2dEvents (EventRegistrar &_registrar) noexcept;
} // namespace Emergence::Celerity

#pragma once

#include <Celerity/Event/EventRegistrar.hpp>
#include <Celerity/Event/Macro.generated.hpp>
#include <Celerity/Standard/UniqueId.hpp>

EMERGENCE_CELERITY_EVENT0_DECLARATION (RenderSceneChangedEvent);

EMERGENCE_CELERITY_EVENT1_DECLARATION (CameraComponentAddedEvent, Emergence::Celerity::UniqueId, objectId);
EMERGENCE_CELERITY_EVENT1_DECLARATION (CameraComponentChangedEvent, Emergence::Celerity::UniqueId, objectId);
EMERGENCE_CELERITY_EVENT2_DECLARATION (
    CameraComponentRemovedEvent, Emergence::Celerity::UniqueId, objectId, void *, implementationHandle);

EMERGENCE_CELERITY_EVENT1_DECLARATION (LightComponentAddedEvent, Emergence::Celerity::UniqueId, lightId);
EMERGENCE_CELERITY_EVENT1_DECLARATION (LightComponentChangedEvent, Emergence::Celerity::UniqueId, lightId);
EMERGENCE_CELERITY_EVENT2_DECLARATION (
    LightComponentRemovedEvent, Emergence::Celerity::UniqueId, lightId, void *, implementationHandle);

EMERGENCE_CELERITY_EVENT1_DECLARATION (StaticModelComponentAddedEvent, Emergence::Celerity::UniqueId, modelId);
EMERGENCE_CELERITY_EVENT1_DECLARATION (StaticModelComponentChangedEvent, Emergence::Celerity::UniqueId, modelId);
EMERGENCE_CELERITY_EVENT2_DECLARATION (
    StaticModelComponentRemovedEvent, Emergence::Celerity::UniqueId, modelId, void *, implementationHandle);

void RegisterRenderEvents (Emergence::Celerity::EventRegistrar &_registrar) noexcept;

#pragma once

#include <Celerity/Event/EventRegistrar.hpp>
#include <Celerity/Event/Macro.generated.hpp>
#include <Celerity/Standard/UniqueId.hpp>

EMERGENCE_CELERITY_EVENT0_DECLARATION (RenderSceneChangedNormalEvent);
EMERGENCE_CELERITY_EVENT0_DECLARATION (RenderSceneChangedCustomToNormalEvent);

EMERGENCE_CELERITY_EVENT1_DECLARATION (CameraComponentAddedNormalEvent, Emergence::Celerity::UniqueId, objectId);
EMERGENCE_CELERITY_EVENT1_DECLARATION (CameraComponentAddedCustomToNormalEvent,
                                       Emergence::Celerity::UniqueId,
                                       objectId);
EMERGENCE_CELERITY_EVENT1_DECLARATION (CameraComponentChangedEvent, Emergence::Celerity::UniqueId, objectId);
EMERGENCE_CELERITY_EVENT2_DECLARATION (
    CameraComponentRemovedEvent, Emergence::Celerity::UniqueId, objectId, void *, implementationHandle);

EMERGENCE_CELERITY_EVENT1_DECLARATION (LightComponentAddedNormalEvent, Emergence::Celerity::UniqueId, lightId);
EMERGENCE_CELERITY_EVENT1_DECLARATION (LightComponentAddedCustomToNormalEvent, Emergence::Celerity::UniqueId, lightId);
EMERGENCE_CELERITY_EVENT1_DECLARATION (LightComponentChangedEvent, Emergence::Celerity::UniqueId, lightId);
EMERGENCE_CELERITY_EVENT2_DECLARATION (
    LightComponentRemovedEvent, Emergence::Celerity::UniqueId, objectId, void *, implementationHandle);

EMERGENCE_CELERITY_EVENT1_DECLARATION (ParticleEffectComponentAddedNormalEvent,
                                       Emergence::Celerity::UniqueId,
                                       effectId);
EMERGENCE_CELERITY_EVENT1_DECLARATION (ParticleEffectComponentAddedCustomToNormalEvent,
                                       Emergence::Celerity::UniqueId,
                                       effectId);
EMERGENCE_CELERITY_EVENT1_DECLARATION (ParticleEffectComponentChangedEvent, Emergence::Celerity::UniqueId, effectId);
EMERGENCE_CELERITY_EVENT2_DECLARATION (
    ParticleEffectComponentRemovedEvent, Emergence::Celerity::UniqueId, objectId, void *, implementationHandle);

EMERGENCE_CELERITY_EVENT1_DECLARATION (StaticModelComponentAddedNormalEvent, Emergence::Celerity::UniqueId, modelId);
EMERGENCE_CELERITY_EVENT1_DECLARATION (StaticModelComponentAddedCustomToNormalEvent,
                                       Emergence::Celerity::UniqueId,
                                       modelId);
EMERGENCE_CELERITY_EVENT1_DECLARATION (StaticModelComponentChangedEvent, Emergence::Celerity::UniqueId, modelId);
EMERGENCE_CELERITY_EVENT2_DECLARATION (
    StaticModelComponentRemovedEvent, Emergence::Celerity::UniqueId, objectId, void *, implementationHandle);

void RegisterRenderEvents (Emergence::Celerity::EventRegistrar &_registrar) noexcept;

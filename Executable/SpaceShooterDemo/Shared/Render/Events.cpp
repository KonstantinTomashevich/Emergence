#include <Render/CameraComponent.hpp>
#include <Render/Events.hpp>
#include <Render/LightComponent.hpp>
#include <Render/ParticleEffectComponent.hpp>
#include <Render/RenderSceneSingleton.hpp>
#include <Render/StaticModelComponent.hpp>

EMERGENCE_CELERITY_EVENT0_IMPLEMENTATION (RenderSceneChangedNormalEvent);
EMERGENCE_CELERITY_EVENT0_IMPLEMENTATION (RenderSceneChangedCustomToNormalEvent);

EMERGENCE_CELERITY_EVENT1_IMPLEMENTATION (CameraComponentAddedNormalEvent, REGULAR, objectId)
EMERGENCE_CELERITY_EVENT1_IMPLEMENTATION (CameraComponentAddedCustomToNormalEvent, REGULAR, objectId)
EMERGENCE_CELERITY_EVENT1_IMPLEMENTATION (CameraComponentChangedEvent, REGULAR, objectId)
EMERGENCE_CELERITY_EVENT2_IMPLEMENTATION (
    CameraComponentRemovedEvent, REGULAR, objectId, POINTER_AS_REGULAR, implementationHandle)

EMERGENCE_CELERITY_EVENT1_IMPLEMENTATION (LightComponentAddedNormalEvent, REGULAR, lightId)
EMERGENCE_CELERITY_EVENT1_IMPLEMENTATION (LightComponentAddedCustomToNormalEvent, REGULAR, lightId)
EMERGENCE_CELERITY_EVENT1_IMPLEMENTATION (LightComponentChangedEvent, REGULAR, lightId)
EMERGENCE_CELERITY_EVENT2_IMPLEMENTATION (
    LightComponentRemovedEvent, REGULAR, objectId, POINTER_AS_REGULAR, implementationHandle)

EMERGENCE_CELERITY_EVENT1_IMPLEMENTATION (ParticleEffectComponentAddedNormalEvent, REGULAR, effectId);
EMERGENCE_CELERITY_EVENT1_IMPLEMENTATION (ParticleEffectComponentAddedCustomToNormalEvent, REGULAR, effectId);
EMERGENCE_CELERITY_EVENT1_IMPLEMENTATION (ParticleEffectComponentChangedEvent, REGULAR, effectId);
EMERGENCE_CELERITY_EVENT2_IMPLEMENTATION (
    ParticleEffectComponentRemovedEvent, REGULAR, objectId, POINTER_AS_REGULAR, implementationHandle);

EMERGENCE_CELERITY_EVENT1_IMPLEMENTATION (StaticModelComponentAddedNormalEvent, REGULAR, modelId)
EMERGENCE_CELERITY_EVENT1_IMPLEMENTATION (StaticModelComponentAddedCustomToNormalEvent, REGULAR, modelId)
EMERGENCE_CELERITY_EVENT1_IMPLEMENTATION (StaticModelComponentChangedEvent, REGULAR, modelId)
EMERGENCE_CELERITY_EVENT2_IMPLEMENTATION (
    StaticModelComponentRemovedEvent, REGULAR, objectId, POINTER_AS_REGULAR, implementationHandle)

void RegisterRenderEvents (Emergence::Celerity::EventRegistrar &_registrar) noexcept
{
    // RenderSceneSingleton

    _registrar.OnChangeEvent (
        {{RenderSceneChangedNormalEvent::Reflect ().mapping, Emergence::Celerity::EventRoute::NORMAL},
         RenderSceneSingleton::Reflect ().mapping,
         {
             RenderSceneSingleton::Reflect ().cameraObjectId,
         },
         {},
         {}});

    _registrar.OnChangeEvent ({{RenderSceneChangedCustomToNormalEvent::Reflect ().mapping,
                                Emergence::Celerity::EventRoute::FROM_CUSTOM_TO_NORMAL},
                               RenderSceneSingleton::Reflect ().mapping,
                               {
                                   RenderSceneSingleton::Reflect ().cameraObjectId,
                               },
                               {},
                               {}});

    // CameraComponent

    _registrar.OnAddEvent (
        {{CameraComponentAddedNormalEvent::Reflect ().mapping, Emergence::Celerity::EventRoute::NORMAL},
         CameraComponent::Reflect ().mapping,
         {{CameraComponent::Reflect ().objectId, CameraComponentAddedNormalEvent::Reflect ().objectId}}});

    _registrar.OnAddEvent (
        {{CameraComponentAddedCustomToNormalEvent::Reflect ().mapping,
          Emergence::Celerity::EventRoute::FROM_CUSTOM_TO_NORMAL},
         CameraComponent::Reflect ().mapping,
         {{CameraComponent::Reflect ().objectId, CameraComponentAddedCustomToNormalEvent::Reflect ().objectId}}});

    _registrar.OnChangeEvent (
        {{CameraComponentChangedEvent::Reflect ().mapping, Emergence::Celerity::EventRoute::NORMAL},
         CameraComponent::Reflect ().mapping,
         {
             CameraComponent::Reflect ().fieldOfViewRad,
         },
         {},
         {{CameraComponent::Reflect ().objectId, CameraComponentAddedNormalEvent::Reflect ().objectId}}});

    _registrar.OnRemoveEvent (
        {{CameraComponentRemovedEvent::Reflect ().mapping, Emergence::Celerity::EventRoute::NORMAL},
         CameraComponent::Reflect ().mapping,
         {{CameraComponent::Reflect ().objectId, CameraComponentRemovedEvent::Reflect ().objectId},
          {CameraComponent::Reflect ().implementationHandle,
           CameraComponentRemovedEvent::Reflect ().implementationHandle}}});

    // LightComponent

    _registrar.OnAddEvent (
        {{LightComponentAddedNormalEvent::Reflect ().mapping, Emergence::Celerity::EventRoute::NORMAL},
         LightComponent::Reflect ().mapping,
         {{LightComponent::Reflect ().lightId, LightComponentAddedNormalEvent::Reflect ().lightId}}});

    _registrar.OnAddEvent (
        {{LightComponentAddedCustomToNormalEvent::Reflect ().mapping,
          Emergence::Celerity::EventRoute::FROM_CUSTOM_TO_NORMAL},
         LightComponent::Reflect ().mapping,
         {{LightComponent::Reflect ().lightId, LightComponentAddedCustomToNormalEvent::Reflect ().lightId}}});

    _registrar.OnChangeEvent (
        {{LightComponentChangedEvent::Reflect ().mapping, Emergence::Celerity::EventRoute::NORMAL},
         LightComponent::Reflect ().mapping,
         {
             LightComponent::Reflect ().type,
             LightComponent::Reflect ().enabled,
             LightComponent::Reflect ().color,
             LightComponent::Reflect ().range,
             LightComponent::Reflect ().spotFieldOfViewRad,
             LightComponent::Reflect ().spotAspectRatio,
         },
         {},
         {{LightComponent::Reflect ().lightId, LightComponentAddedNormalEvent::Reflect ().lightId}}});

    _registrar.OnRemoveEvent (
        {{LightComponentRemovedEvent::Reflect ().mapping, Emergence::Celerity::EventRoute::NORMAL},
         LightComponent::Reflect ().mapping,
         {{LightComponent::Reflect ().objectId, LightComponentRemovedEvent::Reflect ().objectId},
          {LightComponent::Reflect ().implementationHandle,
           LightComponentRemovedEvent::Reflect ().implementationHandle}}});

    // ParticleEffectComponent

    _registrar.OnAddEvent (
        {{ParticleEffectComponentAddedNormalEvent::Reflect ().mapping, Emergence::Celerity::EventRoute::NORMAL},
         ParticleEffectComponent::Reflect ().mapping,
         {{ParticleEffectComponent::Reflect ().effectId,
           ParticleEffectComponentAddedNormalEvent::Reflect ().effectId}}});

    _registrar.OnAddEvent ({{ParticleEffectComponentAddedCustomToNormalEvent::Reflect ().mapping,
                             Emergence::Celerity::EventRoute::FROM_CUSTOM_TO_NORMAL},
                            ParticleEffectComponent::Reflect ().mapping,
                            {{ParticleEffectComponent::Reflect ().effectId,
                              ParticleEffectComponentAddedCustomToNormalEvent::Reflect ().effectId}}});

    _registrar.OnChangeEvent (
        {{ParticleEffectComponentChangedEvent::Reflect ().mapping, Emergence::Celerity::EventRoute::NORMAL},
         ParticleEffectComponent::Reflect ().mapping,
         {
             ParticleEffectComponent::Reflect ().effectName,
             ParticleEffectComponent::Reflect ().playing,
         },
         {},
         {{ParticleEffectComponent::Reflect ().effectId,
           ParticleEffectComponentAddedNormalEvent::Reflect ().effectId}}});

    _registrar.OnRemoveEvent (
        {{ParticleEffectComponentRemovedEvent::Reflect ().mapping, Emergence::Celerity::EventRoute::NORMAL},
         ParticleEffectComponent::Reflect ().mapping,
         {{ParticleEffectComponent::Reflect ().objectId, ParticleEffectComponentRemovedEvent::Reflect ().objectId},
          {ParticleEffectComponent::Reflect ().implementationHandle,
           ParticleEffectComponentRemovedEvent::Reflect ().implementationHandle}}});

    // StaticModelComponent

    _registrar.OnAddEvent (
        {{StaticModelComponentAddedNormalEvent::Reflect ().mapping, Emergence::Celerity::EventRoute::NORMAL},
         StaticModelComponent::Reflect ().mapping,
         {{StaticModelComponent::Reflect ().modelId, StaticModelComponentAddedNormalEvent::Reflect ().modelId}}});

    _registrar.OnAddEvent ({{StaticModelComponentAddedCustomToNormalEvent::Reflect ().mapping,
                             Emergence::Celerity::EventRoute::FROM_CUSTOM_TO_NORMAL},
                            StaticModelComponent::Reflect ().mapping,
                            {{StaticModelComponent::Reflect ().modelId,
                              StaticModelComponentAddedCustomToNormalEvent::Reflect ().modelId}}});

    _registrar.OnChangeEvent (
        {{StaticModelComponentChangedEvent::Reflect ().mapping, Emergence::Celerity::EventRoute::NORMAL},
         StaticModelComponent::Reflect ().mapping,
         {
             StaticModelComponent::Reflect ().modelName,
             StaticModelComponent::Reflect ().materialNamesBlock,
         },
         {},
         {{StaticModelComponent::Reflect ().modelId, StaticModelComponentAddedNormalEvent::Reflect ().modelId}}});

    _registrar.OnRemoveEvent (
        {{StaticModelComponentRemovedEvent::Reflect ().mapping, Emergence::Celerity::EventRoute::NORMAL},
         StaticModelComponent::Reflect ().mapping,
         {{StaticModelComponent::Reflect ().objectId, StaticModelComponentRemovedEvent::Reflect ().objectId},
          {StaticModelComponent::Reflect ().implementationHandle,
           StaticModelComponentRemovedEvent::Reflect ().implementationHandle}}});
}

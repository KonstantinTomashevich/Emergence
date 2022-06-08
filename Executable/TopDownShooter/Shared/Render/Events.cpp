#include <Render/CameraComponent.hpp>
#include <Render/Events.hpp>
#include <Render/LightComponent.hpp>
#include <Render/RenderSceneSingleton.hpp>
#include <Render/StaticModelComponent.hpp>

EMERGENCE_CELERITY_EVENT0_IMPLEMENTATION (RenderSceneChangedEvent);

EMERGENCE_CELERITY_EVENT1_IMPLEMENTATION (CameraComponentAddedEvent, REGULAR, objectId)
EMERGENCE_CELERITY_EVENT1_IMPLEMENTATION (CameraComponentChangedEvent, REGULAR, objectId)
EMERGENCE_CELERITY_EVENT2_IMPLEMENTATION (
    CameraComponentRemovedEvent, REGULAR, objectId, POINTER_AS_REGULAR, implementationHandle)

EMERGENCE_CELERITY_EVENT1_IMPLEMENTATION (LightComponentAddedEvent, REGULAR, lightId)
EMERGENCE_CELERITY_EVENT1_IMPLEMENTATION (LightComponentChangedEvent, REGULAR, lightId)
EMERGENCE_CELERITY_EVENT2_IMPLEMENTATION (
    LightComponentRemovedEvent, REGULAR, objectId, POINTER_AS_REGULAR, implementationHandle)

EMERGENCE_CELERITY_EVENT1_IMPLEMENTATION (StaticModelComponentAddedEvent, REGULAR, modelId)
EMERGENCE_CELERITY_EVENT1_IMPLEMENTATION (StaticModelComponentChangedEvent, REGULAR, modelId)
EMERGENCE_CELERITY_EVENT2_IMPLEMENTATION (
    StaticModelComponentRemovedEvent, REGULAR, objectId, POINTER_AS_REGULAR, implementationHandle)

void RegisterRenderEvents (Emergence::Celerity::EventRegistrar &_registrar) noexcept
{
    // RenderSceneSingleton

    _registrar.OnChangeEvent ({{RenderSceneChangedEvent::Reflect ().mapping, Emergence::Celerity::EventRoute::NORMAL},
                               RenderSceneSingleton::Reflect ().mapping,
                               {
                                   RenderSceneSingleton::Reflect ().cameraObjectId,
                               },
                               {},
                               {}});

    // CameraComponent

    _registrar.OnAddEvent ({{CameraComponentAddedEvent::Reflect ().mapping, Emergence::Celerity::EventRoute::NORMAL},
                            CameraComponent::Reflect ().mapping,
                            {{CameraComponent::Reflect ().objectId, CameraComponentAddedEvent::Reflect ().objectId}}});

    _registrar.OnChangeEvent (
        {{CameraComponentChangedEvent::Reflect ().mapping, Emergence::Celerity::EventRoute::NORMAL},
         CameraComponent::Reflect ().mapping,
         {
             CameraComponent::Reflect ().fieldOfViewRad,
         },
         {},
         {{CameraComponent::Reflect ().objectId, CameraComponentAddedEvent::Reflect ().objectId}}});

    _registrar.OnRemoveEvent (
        {{CameraComponentRemovedEvent::Reflect ().mapping, Emergence::Celerity::EventRoute::NORMAL},
         CameraComponent::Reflect ().mapping,
         {{CameraComponent::Reflect ().objectId, CameraComponentRemovedEvent::Reflect ().objectId},
          {CameraComponent::Reflect ().implementationHandle,
           CameraComponentRemovedEvent::Reflect ().implementationHandle}}});

    // LightComponent

    _registrar.OnAddEvent ({{LightComponentAddedEvent::Reflect ().mapping, Emergence::Celerity::EventRoute::NORMAL},
                            LightComponent::Reflect ().mapping,
                            {{LightComponent::Reflect ().lightId, LightComponentAddedEvent::Reflect ().lightId}}});

    _registrar.OnChangeEvent (
        {{LightComponentChangedEvent::Reflect ().mapping, Emergence::Celerity::EventRoute::NORMAL},
         LightComponent::Reflect ().mapping,
         {
             LightComponent::Reflect ().lightType,
             LightComponent::Reflect ().enabled,
             LightComponent::Reflect ().color,
             LightComponent::Reflect ().range,
             LightComponent::Reflect ().spotFieldOfViewRad,
             LightComponent::Reflect ().spotAspectRatio,
         },
         {},
         {{LightComponent::Reflect ().lightId, LightComponentAddedEvent::Reflect ().lightId}}});

    _registrar.OnRemoveEvent (
        {{LightComponentRemovedEvent::Reflect ().mapping, Emergence::Celerity::EventRoute::NORMAL},
         LightComponent::Reflect ().mapping,
         {{LightComponent::Reflect ().objectId, LightComponentRemovedEvent::Reflect ().objectId},
          {LightComponent::Reflect ().implementationHandle,
           LightComponentRemovedEvent::Reflect ().implementationHandle}}});

    // StaticModelComponent

    _registrar.OnAddEvent (
        {{StaticModelComponentAddedEvent::Reflect ().mapping, Emergence::Celerity::EventRoute::NORMAL},
         StaticModelComponent::Reflect ().mapping,
         {{StaticModelComponent::Reflect ().modelId, StaticModelComponentAddedEvent::Reflect ().modelId}}});

    _registrar.OnChangeEvent (
        {{StaticModelComponentChangedEvent::Reflect ().mapping, Emergence::Celerity::EventRoute::NORMAL},
         StaticModelComponent::Reflect ().mapping,
         {
             StaticModelComponent::Reflect ().modelName,
             StaticModelComponent::Reflect ().materialNamesBlock,
         },
         {},
         {{StaticModelComponent::Reflect ().modelId, StaticModelComponentAddedEvent::Reflect ().modelId}}});

    _registrar.OnRemoveEvent (
        {{StaticModelComponentRemovedEvent::Reflect ().mapping, Emergence::Celerity::EventRoute::NORMAL},
         StaticModelComponent::Reflect ().mapping,
         {{StaticModelComponent::Reflect ().objectId, StaticModelComponentRemovedEvent::Reflect ().objectId},
          {StaticModelComponent::Reflect ().implementationHandle,
           StaticModelComponentRemovedEvent::Reflect ().implementationHandle}}});
}

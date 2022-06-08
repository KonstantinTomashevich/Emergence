#include <Celerity/Model/TimeSingleton.hpp>
#include <Celerity/PipelineBuilderMacros.hpp>

#include <Math/Scalar.hpp>

#include <Render/CameraComponent.hpp>
#include <Render/Events.hpp>
#include <Render/LightComponent.hpp>
#include <Render/RenderSceneSingleton.hpp>
#include <Render/StaticModelComponent.hpp>
#include <Render/Urho3DAccessSingleton.hpp>
#include <Render/Urho3DNodeComponent.hpp>
#include <Render/Urho3DSceneSingleton.hpp>
#include <Render/Urho3DUpdate.hpp>

#include <Shared/Checkpoint.hpp>

#include <Transform/Events.hpp>
#include <Transform/Transform3dComponent.hpp>
#include <Transform/Transform3dWorldAccessor.hpp>
#include <Transform/Transform3dVisualSync.hpp>

BEGIN_MUTING_WARNINGS
#include <Urho3D/Graphics/Camera.h>
#include <Urho3D/Graphics/Light.h>
#include <Urho3D/Graphics/Material.h>
#include <Urho3D/Graphics/Model.h>
#include <Urho3D/Graphics/Octree.h>
#include <Urho3D/Graphics/Renderer.h>
#include <Urho3D/Graphics/StaticModel.h>
#include <Urho3D/Resource/ResourceCache.h>
END_MUTING_WARNINGS

// TODO: Urho3D doesn't support custom allocators at the moment, therefore we're using new for Urho3D objects here.

namespace Urho3DUpdate
{
namespace TaskNames
{
static const Emergence::Memory::UniqueString ENSURE_SCENE_IS_READY {"Urho3DUpdate::EnsureSceneIsReady"};
static const Emergence::Memory::UniqueString CLEANUP_AFTER_TRANSFORM_REMOVAL {
    "Urho3DUpdate::CleanupAfterTransformRemoval"};

static const Emergence::Memory::UniqueString INITIALIZE_NEW_COMPONENTS {"Urho3DUpdate::InitializeNewComponents"};
static const Emergence::Memory::UniqueString APPLY_COMPONENT_CHANGES {"Urho3DUpdate::ApplyComponentChanges"};
static const Emergence::Memory::UniqueString APPLY_COMPONENT_DELETION {"Urho3DUpdate::ApplyComponentDeletion"};
static const Emergence::Memory::UniqueString CLEANUP_UNUSED_NODES {"Urho3DUpdate::CleanupUnusedNodes"};
static const Emergence::Memory::UniqueString UPDATE_SCENE {"Urho3DUpdate::UpdateScene"};
} // namespace TaskNames

static void SyncCamera (const CameraComponent *_camera, Urho3D::Camera *_urho3DCamera) noexcept;

static void SyncLight (const LightComponent *_light, Urho3D::Light *_urho3DLight) noexcept;

static void SyncStaticModel (const StaticModelComponent *_staticModel,
                             Urho3D::StaticModel *_urho3DStaticModel) noexcept;

class Urho3DNodeAccessor final
{
public:
    Urho3DNodeAccessor (Emergence::Celerity::TaskConstructor &_constructor) noexcept;

    Urho3D::Node *StartUsingNode (Emergence::Celerity::UniqueId _objectId) noexcept;

    Urho3D::Node *UnlinkNode (Emergence::Celerity::UniqueId _objectId) noexcept;

private:
    Urho3D::Node *GetNode (Emergence::Celerity::UniqueId _objectId, int32_t _usageDelta) noexcept;

    Urho3D::Node *CreateNode (Emergence::Celerity::UniqueId _objectId, std::size_t _usages) noexcept;

    Emergence::Celerity::ModifySingletonQuery modifyUrho3D;
    Emergence::Celerity::FetchSingletonQuery fetchUrho3DScene;
    Emergence::Celerity::EditValueQuery editNodeByObjectId;
    Emergence::Celerity::InsertLongTermQuery insertNode;
};

Urho3DNodeAccessor::Urho3DNodeAccessor (Emergence::Celerity::TaskConstructor &_constructor) noexcept
    : modifyUrho3D (_constructor.MModifySingleton (Urho3DAccessSingleton)),
      fetchUrho3DScene (_constructor.MFetchSingleton (Urho3DSceneSingleton)),
      editNodeByObjectId (_constructor.MEditValue1F (Urho3DNodeComponent, objectId)),
      insertNode (_constructor.MInsertLongTerm (Urho3DNodeComponent))
{
}

Urho3D::Node *Urho3DNodeAccessor::StartUsingNode (Emergence::Celerity::UniqueId _objectId) noexcept
{
    if (Urho3D::Node *node = GetNode (_objectId, 1))
    {
        return node;
    }

    return CreateNode (_objectId, 1u);
}

Urho3D::Node *Urho3DNodeAccessor::UnlinkNode (Emergence::Celerity::UniqueId _objectId) noexcept
{
    return GetNode (_objectId, -1);
}

Urho3D::Node *Urho3DNodeAccessor::GetNode (Emergence::Celerity::UniqueId _objectId, int32_t _usageDelta) noexcept
{
    auto cursor = editNodeByObjectId.Execute (&_objectId);
    if (auto *component = static_cast<Urho3DNodeComponent *> (*cursor))
    {
        component->usages += _usageDelta;
        assert (component->node);
        return component->node;
    }

    return nullptr;
}

Urho3D::Node *Urho3DNodeAccessor::CreateNode (Emergence::Celerity::UniqueId _objectId, std::size_t _usages) noexcept
{
    auto sceneCursor = fetchUrho3DScene.Execute ();
    const auto *singleton = static_cast<const Urho3DSceneSingleton *> (*sceneCursor);

    auto cursor = insertNode.Execute ();
    auto *component = static_cast<Urho3DNodeComponent *> (++cursor);
    component->objectId = _objectId;

    // TODO: We do not apply transform parent->child hierarchy to Urho3D. Can it cause problems?
    component->node = singleton->scene->CreateChild (Urho3D::String::EMPTY, Urho3D::LOCAL);

    component->usages = _usages;
    return component->node;
}

class SceneInitializer final : public Emergence::Celerity::TaskExecutorBase<SceneInitializer>
{
public:
    SceneInitializer (Emergence::Celerity::TaskConstructor &_constructor, Urho3D::Context *_context) noexcept;

    void Execute () noexcept;

private:
    Urho3D::Context *context;

    Emergence::Celerity::ModifySingletonQuery modifyUrho3D;
    Emergence::Celerity::ModifySingletonQuery modifyUrho3DScene;
};

SceneInitializer::SceneInitializer (Emergence::Celerity::TaskConstructor &_constructor,
                                    Urho3D::Context *_context) noexcept
    : context (_context),
      modifyUrho3D (_constructor.MModifySingleton (Urho3DAccessSingleton)),
      modifyUrho3DScene (_constructor.MModifySingleton (Urho3DSceneSingleton))
{
    _constructor.DependOn (Checkpoint::RENDER_UPDATE_STARTED);
}

void SceneInitializer::Execute () noexcept
{
    auto cursor = modifyUrho3DScene.Execute ();
    auto *sceneSingleton = static_cast<Urho3DSceneSingleton *> (*cursor);

    if (!sceneSingleton->scene)
    {
        sceneSingleton->scene = new Urho3D::Scene {context};
        sceneSingleton->scene->SetUpdateEnabled (false);
        sceneSingleton->scene->CreateComponent<Urho3D::Octree> ();
    }
}

class TransformEventProcessor final : public Emergence::Celerity::TaskExecutorBase<TransformEventProcessor>
{
public:
    TransformEventProcessor (Emergence::Celerity::TaskConstructor &_constructor) noexcept;

    void Execute () noexcept;

private:
    Emergence::Celerity::ModifySingletonQuery modifyUrho3D;
    Emergence::Celerity::FetchSequenceQuery fetchTransformRemovedNormalEvents;
    Emergence::Celerity::FetchSequenceQuery fetchTransformRemovedFixedToNormalEvents;

    Emergence::Celerity::RemoveValueQuery removeCameraByObjectId;
    Emergence::Celerity::RemoveValueQuery removeLightByObjectId;
    Emergence::Celerity::RemoveValueQuery removeStaticModelByObjectId;
    Emergence::Celerity::RemoveValueQuery removeNodeByObjectId;
};

TransformEventProcessor::TransformEventProcessor (Emergence::Celerity::TaskConstructor &_constructor) noexcept
    : modifyUrho3D (_constructor.MModifySingleton (Urho3DAccessSingleton)),
      fetchTransformRemovedNormalEvents (
          _constructor.MFetchSequence (Emergence::Transform::Transform3dComponentRemovedNormalEvent)),
      fetchTransformRemovedFixedToNormalEvents (
          _constructor.MFetchSequence (Emergence::Transform::Transform3dComponentRemovedFixedToNormalEvent)),

      removeCameraByObjectId (_constructor.MRemoveValue1F (CameraComponent, objectId)),
      removeLightByObjectId (_constructor.MRemoveValue1F (LightComponent, objectId)),
      removeStaticModelByObjectId (_constructor.MRemoveValue1F (StaticModelComponent, objectId)),
      removeNodeByObjectId (_constructor.MRemoveValue1F (Urho3DNodeComponent, objectId))
{
    _constructor.DependOn (TaskNames::ENSURE_SCENE_IS_READY);
}

void TransformEventProcessor::Execute () noexcept
{
    auto cleanup = [this] (Emergence::Celerity::UniqueId _objectId)
    {
        auto cameraCursor = removeCameraByObjectId.Execute (&_objectId);
        if (cameraCursor.ReadConst ())
        {
            ~cameraCursor;
        }

        auto nodeCursor = removeNodeByObjectId.Execute (&_objectId);
        if (const auto *node = static_cast<const Urho3DNodeComponent *> (nodeCursor.ReadConst ()))
        {
            if (node->node)
            {
                node->node->Remove ();
            }

            ~nodeCursor;
        }

        for (auto lightCursor = removeLightByObjectId.Execute (&_objectId); lightCursor.ReadConst (); ++lightCursor)
        {
            ~lightCursor;
        }

        for (auto modelCursor = removeStaticModelByObjectId.Execute (&_objectId); modelCursor.ReadConst ();
             ++modelCursor)
        {
            ~modelCursor;
        }
    };

    for (auto eventCursor = fetchTransformRemovedNormalEvents.Execute ();
         const auto *event =
             static_cast<const Emergence::Transform::Transform3dComponentRemovedNormalEvent *> (*eventCursor);
         ++eventCursor)
    {
        cleanup (event->objectId);
    }

    for (auto eventCursor = fetchTransformRemovedFixedToNormalEvents.Execute ();
         const auto *event =
             static_cast<const Emergence::Transform::Transform3dComponentRemovedFixedToNormalEvent *> (*eventCursor);
         ++eventCursor)
    {
        cleanup (event->objectId);
    }
}

class ComponentInitializer : public Emergence::Celerity::TaskExecutorBase<ComponentInitializer>
{
public:
    ComponentInitializer (Emergence::Celerity::TaskConstructor &_constructor) noexcept;

    void Execute ();

private:
    void InitializeCameras () noexcept;

    void InitializeLights () noexcept;

    void InitializeModels () noexcept;

    Emergence::Celerity::ModifySingletonQuery modifyUrho3D;
    Urho3DNodeAccessor nodeAccessor;

    Emergence::Celerity::EditValueQuery editCameraByObjectId;
    Emergence::Celerity::FetchSequenceQuery fetchCameraAddedEvents;

    Emergence::Celerity::EditValueQuery editLightByLightId;
    Emergence::Celerity::FetchSequenceQuery fetchLightAddedEvents;

    Emergence::Celerity::EditValueQuery editStaticModelByModelId;
    Emergence::Celerity::FetchSequenceQuery fetchStaticModelAddedEvents;
};

ComponentInitializer::ComponentInitializer (Emergence::Celerity::TaskConstructor &_constructor) noexcept
    : modifyUrho3D (_constructor.MModifySingleton (Urho3DAccessSingleton)),
      nodeAccessor (_constructor),

      editCameraByObjectId (_constructor.MEditValue1F (CameraComponent, objectId)),
      fetchCameraAddedEvents (_constructor.MFetchSequence (CameraComponentAddedEvent)),

      editLightByLightId (_constructor.MEditValue1F (LightComponent, lightId)),
      fetchLightAddedEvents (_constructor.MFetchSequence (LightComponentAddedEvent)),

      editStaticModelByModelId (_constructor.MEditValue1F (StaticModelComponent, modelId)),
      fetchStaticModelAddedEvents (_constructor.MFetchSequence (StaticModelComponentAddedEvent))
{
    _constructor.DependOn (TaskNames::CLEANUP_AFTER_TRANSFORM_REMOVAL);
}

void ComponentInitializer::Execute ()
{
    InitializeCameras ();
    InitializeLights ();
    InitializeModels ();
}

void ComponentInitializer::InitializeCameras () noexcept
{
    for (auto eventCursor = fetchCameraAddedEvents.Execute ();
         const auto *event = static_cast<const CameraComponentAddedEvent *> (*eventCursor); ++eventCursor)
    {
        auto cameraCursor = editCameraByObjectId.Execute (&event->objectId);
        if (auto *camera = static_cast<CameraComponent *> (*cameraCursor))
        {
            Urho3D::Node *node = nodeAccessor.StartUsingNode (camera->objectId);
            auto *urho3DCamera = node->CreateComponent<Urho3D::Camera> ();
            SyncCamera (camera, urho3DCamera);
            camera->implementationHandle = urho3DCamera;
        }
    }
}

void ComponentInitializer::InitializeLights () noexcept
{
    for (auto eventCursor = fetchLightAddedEvents.Execute ();
         const auto *event = static_cast<const LightComponentAddedEvent *> (*eventCursor); ++eventCursor)
    {
        auto lightCursor = editLightByLightId.Execute (&event->lightId);
        if (auto *light = static_cast<LightComponent *> (*lightCursor))
        {
            Urho3D::Node *node = nodeAccessor.StartUsingNode (light->objectId);
            auto *urho3DLight = node->CreateComponent<Urho3D::Light> ();
            SyncLight (light, urho3DLight);
            light->implementationHandle = urho3DLight;
        }
    }
}

void ComponentInitializer::InitializeModels () noexcept
{
    for (auto eventCursor = fetchStaticModelAddedEvents.Execute ();
         const auto *event = static_cast<const StaticModelComponentAddedEvent *> (*eventCursor); ++eventCursor)
    {
        auto modelCursor = editStaticModelByModelId.Execute (&event->modelId);
        if (auto *staticModel = static_cast<StaticModelComponent *> (*modelCursor))
        {
            Urho3D::Node *node = nodeAccessor.StartUsingNode (staticModel->objectId);
            auto *urho3DStaticModel = node->CreateComponent<Urho3D::StaticModel> ();
            SyncStaticModel (staticModel, urho3DStaticModel);
            staticModel->implementationHandle = urho3DStaticModel;
        }
    }
}

class ComponentSynchronizer : public Emergence::Celerity::TaskExecutorBase<ComponentSynchronizer>
{
public:
    ComponentSynchronizer (Emergence::Celerity::TaskConstructor &_constructor) noexcept;

    void Execute ();

private:
    void SyncCameras () noexcept;

    void SyncLights () noexcept;

    void SyncModels () noexcept;

    Emergence::Celerity::ModifySingletonQuery modifyUrho3D;

    Emergence::Celerity::FetchValueQuery fetchCameraByObjectId;
    Emergence::Celerity::FetchSequenceQuery fetchCameraChangedEvents;

    Emergence::Celerity::FetchValueQuery fetchLightByLightId;
    Emergence::Celerity::FetchSequenceQuery fetchLightChangedEvents;

    Emergence::Celerity::FetchValueQuery fetchStaticModelByModelId;
    Emergence::Celerity::FetchSequenceQuery fetchStaticModelChangedEvents;
};

ComponentSynchronizer::ComponentSynchronizer (Emergence::Celerity::TaskConstructor &_constructor) noexcept
    : modifyUrho3D (_constructor.MModifySingleton (Urho3DAccessSingleton)),

      fetchCameraByObjectId (_constructor.MFetchValue1F (CameraComponent, objectId)),
      fetchCameraChangedEvents (_constructor.MFetchSequence (CameraComponentChangedEvent)),

      fetchLightByLightId (_constructor.MFetchValue1F (LightComponent, lightId)),
      fetchLightChangedEvents (_constructor.MFetchSequence (LightComponentChangedEvent)),

      fetchStaticModelByModelId (_constructor.MFetchValue1F (StaticModelComponent, modelId)),
      fetchStaticModelChangedEvents (_constructor.MFetchSequence (StaticModelComponentChangedEvent))
{
    _constructor.DependOn (TaskNames::INITIALIZE_NEW_COMPONENTS);
}

void ComponentSynchronizer::Execute ()
{
    SyncCameras ();
    SyncLights ();
    SyncModels ();
}

void ComponentSynchronizer::SyncCameras () noexcept
{
    for (auto eventCursor = fetchCameraChangedEvents.Execute ();
         const auto *event = static_cast<const CameraComponentChangedEvent *> (*eventCursor); ++eventCursor)
    {
        auto cameraCursor = fetchCameraByObjectId.Execute (&event->objectId);
        if (const auto *camera = static_cast<const CameraComponent *> (*cameraCursor))
        {
            SyncCamera (camera, static_cast<Urho3D::Camera *> (camera->implementationHandle));
        }
    }
}

void ComponentSynchronizer::SyncLights () noexcept
{
    for (auto eventCursor = fetchLightChangedEvents.Execute ();
         const auto *event = static_cast<const LightComponentChangedEvent *> (*eventCursor); ++eventCursor)
    {
        auto lightCursor = fetchLightByLightId.Execute (&event->lightId);
        if (const auto *light = static_cast<const LightComponent *> (*lightCursor))
        {
            SyncLight (light, static_cast<Urho3D::Light *> (light->implementationHandle));
        }
    }
}

void ComponentSynchronizer::SyncModels () noexcept
{
    for (auto eventCursor = fetchStaticModelChangedEvents.Execute ();
         const auto *event = static_cast<const StaticModelComponentChangedEvent *> (*eventCursor); ++eventCursor)
    {
        auto staticModelCursor = fetchStaticModelByModelId.Execute (&event->modelId);
        if (const auto *staticModel = static_cast<const StaticModelComponent *> (*staticModelCursor))
        {
            SyncStaticModel (staticModel, static_cast<Urho3D::StaticModel *> (staticModel->implementationHandle));
        }
    }
}

class ComponentDeleter : public Emergence::Celerity::TaskExecutorBase<ComponentDeleter>
{
public:
    ComponentDeleter (Emergence::Celerity::TaskConstructor &_constructor) noexcept;

    void Execute ();

private:
    Emergence::Celerity::ModifySingletonQuery modifyUrho3D;
    Emergence::Celerity::ModifySingletonQuery modifyRenderScene;
    Urho3DNodeAccessor nodeAccessor;

    Emergence::Celerity::FetchSequenceQuery fetchCameraRemovedEvents;
    Emergence::Celerity::FetchSequenceQuery fetchLightRemovedEvents;
    Emergence::Celerity::FetchSequenceQuery fetchStaticModelRemovedEvents;
};

ComponentDeleter::ComponentDeleter (Emergence::Celerity::TaskConstructor &_constructor) noexcept
    : modifyUrho3D (_constructor.MModifySingleton (Urho3DAccessSingleton)),
      modifyRenderScene (_constructor.MModifySingleton (RenderSceneSingleton)),
      nodeAccessor (_constructor),

      fetchCameraRemovedEvents (_constructor.MFetchSequence (CameraComponentRemovedEvent)),
      fetchLightRemovedEvents (_constructor.MFetchSequence (LightComponentRemovedEvent)),
      fetchStaticModelRemovedEvents (_constructor.MFetchSequence (StaticModelComponentRemovedEvent))
{
    _constructor.DependOn (TaskNames::APPLY_COMPONENT_CHANGES);
}

void ComponentDeleter::Execute ()
{
    auto renderSceneCursor = modifyRenderScene.Execute ();
    auto *renderScene = static_cast<RenderSceneSingleton *> (*renderSceneCursor);

    for (auto eventCursor = fetchCameraRemovedEvents.Execute ();
         const auto *event = static_cast<const CameraComponentRemovedEvent *> (*eventCursor); ++eventCursor)
    {
        if (event->objectId == renderScene->cameraObjectId)
        {
            renderScene->cameraObjectId = Emergence::Celerity::INVALID_UNIQUE_ID;
        }

        Urho3D::Node *node = nodeAccessor.UnlinkNode (event->objectId);
        if (node && event->implementationHandle)
        {
            node->RemoveComponent (static_cast<Urho3D::Camera *> (event->implementationHandle));
        }
    }

    for (auto eventCursor = fetchLightRemovedEvents.Execute ();
         const auto *event = static_cast<const LightComponentRemovedEvent *> (*eventCursor); ++eventCursor)
    {
        Urho3D::Node *node = nodeAccessor.UnlinkNode (event->objectId);
        if (node && event->implementationHandle)
        {
            node->RemoveComponent (static_cast<Urho3D::Light *> (event->implementationHandle));
        }
    }

    for (auto eventCursor = fetchStaticModelRemovedEvents.Execute ();
         const auto *event = static_cast<const StaticModelComponentRemovedEvent *> (*eventCursor); ++eventCursor)
    {
        Urho3D::Node *node = nodeAccessor.UnlinkNode (event->objectId);
        if (node && event->implementationHandle)
        {
            node->RemoveComponent (static_cast<Urho3D::StaticModel *> (event->implementationHandle));
        }
    }
}

class Urho3DNodeCleaner final : public Emergence::Celerity::TaskExecutorBase<Urho3DNodeCleaner>
{
public:
    Urho3DNodeCleaner (Emergence::Celerity::TaskConstructor &_constructor) noexcept;

    void Execute ();

private:
    Emergence::Celerity::ModifySingletonQuery modifyUrho3D;
    Emergence::Celerity::RemoveSignalQuery removeUnusedNodes;
};

Urho3DNodeCleaner::Urho3DNodeCleaner (Emergence::Celerity::TaskConstructor &_constructor) noexcept
    : modifyUrho3D (_constructor.MModifySingleton (Urho3DAccessSingleton)),
      removeUnusedNodes (_constructor.MRemoveSignal (Urho3DNodeComponent, usages, 0u))
{
    _constructor.DependOn (TaskNames::APPLY_COMPONENT_DELETION);
}

void Urho3DNodeCleaner::Execute ()
{
    for (auto cursor = removeUnusedNodes.Execute ();
         const auto *component = static_cast<const Urho3DNodeComponent *> (cursor.ReadConst ()); ~cursor)
    {
        component->node->Remove ();
    }
}

class SceneUpdater final : public Emergence::Celerity::TaskExecutorBase<SceneUpdater>
{
public:
    SceneUpdater (Emergence::Celerity::TaskConstructor &_constructor) noexcept;

    void Execute ();

private:
    void UpdateTransforms () noexcept;

    void ApplyRenderSceneChanges () noexcept;

    void UpdateUrho3DScene () noexcept;

    Emergence::Celerity::ModifySingletonQuery modifyUrho3D;
    Emergence::Celerity::FetchSingletonQuery fetchTime;
    Emergence::Celerity::FetchSingletonQuery fetchUrho3DScene;
    Emergence::Celerity::FetchSingletonQuery fetchRenderScene;

    Emergence::Celerity::FetchAscendingRangeQuery fetchUrho3DNodes;
    Emergence::Celerity::FetchValueQuery fetchTransformByObjectId;
    Emergence::Transform::Transform3dWorldAccessor transformWorldAccessor;

    Emergence::Celerity::FetchSequenceQuery fetchRenderSceneChangedEvents;
    Emergence::Celerity::FetchValueQuery fetchCameraByObjectId;
};

SceneUpdater::SceneUpdater (Emergence::Celerity::TaskConstructor &_constructor) noexcept
    : modifyUrho3D (_constructor.MModifySingleton (Urho3DAccessSingleton)),
      fetchTime (_constructor.MFetchSingleton (Emergence::Celerity::TimeSingleton)),
      fetchUrho3DScene (_constructor.MFetchSingleton (Urho3DSceneSingleton)),
      fetchRenderScene (_constructor.MFetchSingleton (RenderSceneSingleton)),

      fetchUrho3DNodes (_constructor.MFetchAscendingRange (Urho3DNodeComponent, objectId)),
      fetchTransformByObjectId (_constructor.MFetchValue1F (Emergence::Transform::Transform3dComponent, objectId)),
      transformWorldAccessor (_constructor),

      fetchRenderSceneChangedEvents (_constructor.MFetchSequence (RenderSceneChangedEvent)),
      fetchCameraByObjectId (_constructor.MFetchValue1F (CameraComponent, objectId))
{
    _constructor.DependOn (Emergence::Transform::VisualSync::Checkpoint::SYNC_FINISHED);
    _constructor.DependOn (TaskNames::CLEANUP_UNUSED_NODES);
    _constructor.MakeDependencyOf (Checkpoint::RENDER_UPDATE_FINISHED);
}

void SceneUpdater::Execute ()
{
    UpdateTransforms ();
    ApplyRenderSceneChanges ();
    UpdateUrho3DScene ();
}

void SceneUpdater::UpdateTransforms () noexcept
{
    // Updating transform for all nodes is not very efficient, but it is ok for the first demo.

    for (auto nodeCursor = fetchUrho3DNodes.Execute (nullptr, nullptr);
         const auto *node = static_cast<const Urho3DNodeComponent *> (*nodeCursor); ++nodeCursor)
    {
        auto transformCursor = fetchTransformByObjectId.Execute (&node->objectId);
        const auto *transform = static_cast<const Emergence::Transform::Transform3dComponent *> (*transformCursor);

        if (!transform)
        {
            continue;
        }

        const Emergence::Math::Transform3d &worldTransform =
            transform->GetVisualWorldTransform (transformWorldAccessor);

        node->node->SetTransform (
            {worldTransform.translation.x, worldTransform.translation.y, worldTransform.translation.z},
            {worldTransform.rotation.w, worldTransform.rotation.x, worldTransform.rotation.y,
             worldTransform.rotation.z},
            {worldTransform.scale.x, worldTransform.scale.y, worldTransform.scale.z});
    }
}

void SceneUpdater::ApplyRenderSceneChanges () noexcept
{
    auto eventCursor = fetchRenderSceneChangedEvents.Execute ();
    if (!*eventCursor)
    {
        return;
    }

    auto renderSceneCursor = fetchRenderScene.Execute ();
    const auto *renderScene = static_cast<const RenderSceneSingleton *> (*renderSceneCursor);

    auto cameraCursor = fetchCameraByObjectId.Execute (&renderScene->cameraObjectId);
    const auto *camera = static_cast<const CameraComponent *> (*cameraCursor);

    auto urho3DSceneCursor = fetchUrho3DScene.Execute ();
    const auto *urho3DScene = static_cast<const Urho3DSceneSingleton *> (*urho3DSceneCursor);

    auto *renderer = urho3DScene->scene->GetContext ()->GetSubsystem<Urho3D::Renderer> ();
    if (camera && camera->implementationHandle)
    {
        renderer->SetViewport (0, new Urho3D::Viewport (urho3DScene->scene->GetContext (), urho3DScene->scene,
                                                        static_cast<Urho3D::Camera *> (camera->implementationHandle)));
    }
    else
    {
        renderer->SetViewport (0, nullptr);
    }
}

void SceneUpdater::UpdateUrho3DScene () noexcept
{
    auto timeCursor = fetchTime.Execute ();
    const auto *time = static_cast<const Emergence::Celerity::TimeSingleton *> (*timeCursor);

    auto urho3DSceneCursor = fetchUrho3DScene.Execute ();
    const auto *urho3DScene = static_cast<const Urho3DSceneSingleton *> (*urho3DSceneCursor);
    urho3DScene->scene->Update (time->normalDurationS);
}

static void SyncCamera (const CameraComponent *_camera, Urho3D::Camera *_urho3DCamera) noexcept
{
    _urho3DCamera->SetFov (Emergence::Math::ToDegrees (_camera->fieldOfViewRad));
}

static void SyncLight (const LightComponent *_light, Urho3D::Light *_urho3DLight) noexcept
{
    switch (_light->lightType)
    {
    case LightType::DIRECTIONAL:
        _urho3DLight->SetLightType (Urho3D::LIGHT_DIRECTIONAL);
        break;
    case LightType::POINT:
        _urho3DLight->SetLightType (Urho3D::LIGHT_POINT);
        break;
    case LightType::SPOT:
        _urho3DLight->SetLightType (Urho3D::LIGHT_SPOT);
        break;
    }

    _urho3DLight->SetEnabled (_light->enabled);
    _urho3DLight->SetColor ({_light->color.r, _light->color.g, _light->color.b, _light->color.a});
    _urho3DLight->SetRange (_light->range);
    _urho3DLight->SetFov (Emergence::Math::ToDegrees (_light->spotFieldOfViewRad));
    _urho3DLight->SetAspectRatio (_light->spotAspectRatio);
}

static void SyncStaticModel (const StaticModelComponent *_staticModel, Urho3D::StaticModel *_urho3DStaticModel) noexcept
{
    auto *cache = _urho3DStaticModel->GetContext ()->GetSubsystem<Urho3D::ResourceCache> ();
    if (*_staticModel->modelName)
    {
        _urho3DStaticModel->SetModel (cache->GetResource<Urho3D::Model> (*_staticModel->modelName));
    }
    else
    {
        _urho3DStaticModel->SetModel (nullptr);
    }

    const std::size_t materialCount =
        std::min (_staticModel->materialNames.size (), static_cast<size_t> (_urho3DStaticModel->GetBatches ().Size ()));

    for (std::size_t index = 0u; index < materialCount; ++index)
    {
        if (*_staticModel->materialNames[index])
        {
            _urho3DStaticModel->SetMaterial (
                index, cache->GetResource<Urho3D::Material> (*_staticModel->materialNames[index]));
        }
        else
        {
            _urho3DStaticModel->SetMaterial (index, nullptr);
        }
    }
}

void AddToNormalUpdate (Urho3D::Context *_context, Emergence::Celerity::PipelineBuilder &_pipelineBuilder) noexcept
{
    _pipelineBuilder.AddTask (TaskNames::ENSURE_SCENE_IS_READY).SetExecutor<SceneInitializer> (_context);
    _pipelineBuilder.AddTask (TaskNames::CLEANUP_AFTER_TRANSFORM_REMOVAL).SetExecutor<TransformEventProcessor> ();
    _pipelineBuilder.AddTask (TaskNames::INITIALIZE_NEW_COMPONENTS).SetExecutor<ComponentInitializer> ();
    _pipelineBuilder.AddTask (TaskNames::APPLY_COMPONENT_CHANGES).SetExecutor<ComponentSynchronizer> ();
    _pipelineBuilder.AddTask (TaskNames::APPLY_COMPONENT_DELETION).SetExecutor<ComponentDeleter> ();
    _pipelineBuilder.AddTask (TaskNames::CLEANUP_UNUSED_NODES).SetExecutor<Urho3DNodeCleaner> ();
    _pipelineBuilder.AddTask (TaskNames::UPDATE_SCENE).SetExecutor<SceneUpdater> ();
}
} // namespace Urho3DUpdate

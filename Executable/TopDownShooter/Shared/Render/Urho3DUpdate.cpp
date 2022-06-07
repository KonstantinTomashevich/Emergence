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

BEGIN_MUTING_WARNINGS
#include <Urho3D/Graphics/Camera.h>
#include <Urho3D/Graphics/Light.h>
#include <Urho3D/Graphics/Material.h>
#include <Urho3D/Graphics/Model.h>
#include <Urho3D/Graphics/Octree.h>
#include <Urho3D/Graphics/StaticModel.h>
#include <Urho3D/Resource/ResourceCache.h>
END_MUTING_WARNINGS

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
static const Emergence::Memory::UniqueString CLEAR_UNUSED_NODES {"Urho3DUpdate::ClearUnusedNodes"};
static const Emergence::Memory::UniqueString UPDATE_SCENE {"Urho3DUpdate::UpdateScene"};
} // namespace TaskNames

static void SyncCamera (CameraComponent *_camera, Urho3D::Camera *_urho3DCamera) noexcept;

static void SyncLight (LightComponent *_light, Urho3D::Light *_urho3DLight) noexcept;

static void SyncStaticModel (StaticModelComponent *_staticModel, Urho3D::StaticModel *_urho3DStaticModel) noexcept;

class Urho3DNodeAccessor final
{
public:
    Urho3DNodeAccessor (Emergence::Celerity::TaskConstructor &_constructor) noexcept;

    Urho3D::Node *StartUsingNode (Emergence::Celerity::UniqueId _objectId) noexcept;

    Urho3D::Node *RequestNode (Emergence::Celerity::UniqueId _objectId) noexcept;

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

Urho3D::Node *Urho3DNodeAccessor::RequestNode (Emergence::Celerity::UniqueId _objectId) noexcept
{
    if (Urho3D::Node *node = GetNode (_objectId, 0))
    {
        return node;
    }

    return CreateNode (_objectId, 0u);
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

static void SyncCamera (CameraComponent *_camera, Urho3D::Camera *_urho3DCamera) noexcept
{
    _urho3DCamera->SetFov (Emergence::Math::ToDegrees (_camera->fieldOfViewRad));
}

static void SyncLight (LightComponent *_light, Urho3D::Light *_urho3DLight) noexcept
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

static void SyncStaticModel (StaticModelComponent *_staticModel, Urho3D::StaticModel *_urho3DStaticModel) noexcept
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

    for (std::size_t index = 0u; index < _staticModel->materialNames.size (); ++index)
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
    // TODO: Implement.
}
} // namespace Urho3DUpdate

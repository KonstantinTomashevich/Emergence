#include <Celerity/Model/TimeSingleton.hpp>
#include <Celerity/PipelineBuilderMacros.hpp>
#include <Celerity/Transform/Events.hpp>
#include <Celerity/Transform/Transform3dComponent.hpp>
#include <Celerity/Transform/Transform3dHierarchyCleanup.hpp>
#include <Celerity/Transform/Transform3dVisualSync.hpp>
#include <Celerity/Transform/Transform3dWorldAccessor.hpp>

#include <Math/Scalar.hpp>

#include <Render/CameraComponent.hpp>
#include <Render/Events.hpp>
#include <Render/LightComponent.hpp>
#include <Render/ParticleEffectComponent.hpp>
#include <Render/RenderSceneSingleton.hpp>
#include <Render/StaticModelComponent.hpp>
#include <Render/Urho3DAccessSingleton.hpp>
#include <Render/Urho3DNodeComponent.hpp>
#include <Render/Urho3DSceneSingleton.hpp>
#include <Render/Urho3DUpdate.hpp>

#include <Shared/Checkpoint.hpp>

BEGIN_MUTING_WARNINGS
#include <Urho3D/Graphics/Camera.h>
#include <Urho3D/Graphics/Light.h>
#include <Urho3D/Graphics/Material.h>
#include <Urho3D/Graphics/Model.h>
#include <Urho3D/Graphics/Octree.h>
#include <Urho3D/Graphics/ParticleEffect.h>
#include <Urho3D/Graphics/ParticleEmitter.h>
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
static const Emergence::Memory::UniqueString INITIALIZE_NEW_COMPONENTS {"Urho3DUpdate::InitializeNewComponents"};
static const Emergence::Memory::UniqueString APPLY_COMPONENT_CHANGES {"Urho3DUpdate::ApplyComponentChanges"};
static const Emergence::Memory::UniqueString APPLY_COMPONENT_DELETION {"Urho3DUpdate::ApplyComponentDeletion"};
static const Emergence::Memory::UniqueString CLEANUP_UNUSED_NODES {"Urho3DUpdate::CleanupUnusedNodes"};
static const Emergence::Memory::UniqueString UPDATE_SCENE {"Urho3DUpdate::UpdateScene"};
} // namespace TaskNames

static void SyncCamera (const CameraComponent *_camera, Urho3D::Camera *_urho3DCamera) noexcept;

static void SyncLight (const LightComponent *_light, Urho3D::Light *_urho3DLight) noexcept;

static void SyncParticleEffect (const ParticleEffectComponent *_effect,
                                Urho3D::ParticleEmitter *_urho3DEffect) noexcept;

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
    : modifyUrho3D (MODIFY_SINGLETON (Urho3DAccessSingleton)),
      fetchUrho3DScene (FETCH_SINGLETON (Urho3DSceneSingleton)),
      editNodeByObjectId (EDIT_VALUE_1F (Urho3DNodeComponent, objectId)),
      insertNode (INSERT_LONG_TERM (Urho3DNodeComponent))
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
      modifyUrho3D (MODIFY_SINGLETON (Urho3DAccessSingleton)),
      modifyUrho3DScene (MODIFY_SINGLETON (Urho3DSceneSingleton))
{
    _constructor.DependOn (Checkpoint::RENDER_UPDATE_STARTED);
    _constructor.MakeDependencyOf (Emergence::Celerity::HierarchyCleanup::Checkpoint::DETACHED_REMOVAL_STARTED);
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

class ComponentInitializer : public Emergence::Celerity::TaskExecutorBase<ComponentInitializer>
{
public:
    ComponentInitializer (Emergence::Celerity::TaskConstructor &_constructor) noexcept;

    void Execute ();

private:
    void InitializeCameras () noexcept;

    void InitializeLights () noexcept;

    void InitializeEffects () noexcept;

    void InitializeModels () noexcept;

    Emergence::Celerity::ModifySingletonQuery modifyUrho3D;
    Urho3DNodeAccessor nodeAccessor;

    Emergence::Celerity::EditValueQuery editCameraByObjectId;
    Emergence::Celerity::FetchSequenceQuery fetchCameraAddedNormalEvents;
    Emergence::Celerity::FetchSequenceQuery fetchCameraAddedCustomEvents;

    Emergence::Celerity::EditValueQuery editLightByLightId;
    Emergence::Celerity::FetchSequenceQuery fetchLightAddedNormalEvents;
    Emergence::Celerity::FetchSequenceQuery fetchLightAddedCustomEvents;

    Emergence::Celerity::EditValueQuery editParticleEffectByEffectId;
    Emergence::Celerity::FetchSequenceQuery fetchParticleEffectAddedNormalEvents;
    Emergence::Celerity::FetchSequenceQuery fetchParticleEffectAddedCustomEvents;

    Emergence::Celerity::EditValueQuery editStaticModelByModelId;
    Emergence::Celerity::FetchSequenceQuery fetchStaticModelAddedNormalEvents;
    Emergence::Celerity::FetchSequenceQuery fetchStaticModelAddedCustomEvents;
};

ComponentInitializer::ComponentInitializer (Emergence::Celerity::TaskConstructor &_constructor) noexcept
    : modifyUrho3D (MODIFY_SINGLETON (Urho3DAccessSingleton)),
      nodeAccessor (_constructor),

      editCameraByObjectId (EDIT_VALUE_1F (CameraComponent, objectId)),
      fetchCameraAddedNormalEvents (FETCH_SEQUENCE (CameraComponentAddedNormalEvent)),
      fetchCameraAddedCustomEvents (FETCH_SEQUENCE (CameraComponentAddedCustomToNormalEvent)),

      editLightByLightId (EDIT_VALUE_1F (LightComponent, lightId)),
      fetchLightAddedNormalEvents (FETCH_SEQUENCE (LightComponentAddedNormalEvent)),
      fetchLightAddedCustomEvents (FETCH_SEQUENCE (LightComponentAddedCustomToNormalEvent)),

      editParticleEffectByEffectId (EDIT_VALUE_1F (ParticleEffectComponent, effectId)),
      fetchParticleEffectAddedNormalEvents (FETCH_SEQUENCE (ParticleEffectComponentAddedNormalEvent)),
      fetchParticleEffectAddedCustomEvents (FETCH_SEQUENCE (ParticleEffectComponentAddedCustomToNormalEvent)),

      editStaticModelByModelId (EDIT_VALUE_1F (StaticModelComponent, modelId)),
      fetchStaticModelAddedNormalEvents (FETCH_SEQUENCE (StaticModelComponentAddedNormalEvent)),
      fetchStaticModelAddedCustomEvents (FETCH_SEQUENCE (StaticModelComponentAddedCustomToNormalEvent))
{
    _constructor.DependOn (TaskNames::ENSURE_SCENE_IS_READY);
}

void ComponentInitializer::Execute ()
{
    InitializeCameras ();
    InitializeLights ();
    InitializeEffects ();
    InitializeModels ();
}

#define INITIALIZE_URHO3D_OBJECT(Source, Type)                                                                         \
    [this, Source] ()                                                                                                  \
    {                                                                                                                  \
        Urho3D::Node *node = nodeAccessor.StartUsingNode ((Source)->objectId);                                         \
        auto *object = node->CreateComponent<Urho3D::Type> ();                                                         \
        Sync##Type (Source, object);                                                                                   \
        return object;                                                                                                 \
    }()

void ComponentInitializer::InitializeCameras () noexcept
{
    auto initialize = [this] (Emergence::Celerity::UniqueId _objectId)
    {
        auto cameraCursor = editCameraByObjectId.Execute (&_objectId);
        if (auto *camera = static_cast<CameraComponent *> (*cameraCursor))
        {
            camera->implementationHandle = INITIALIZE_URHO3D_OBJECT (camera, Camera);
        }
    };

    for (auto eventCursor = fetchCameraAddedNormalEvents.Execute ();
         const auto *event = static_cast<const CameraComponentAddedNormalEvent *> (*eventCursor); ++eventCursor)
    {
        initialize (event->objectId);
    }

    for (auto eventCursor = fetchCameraAddedCustomEvents.Execute ();
         const auto *event = static_cast<const CameraComponentAddedCustomToNormalEvent *> (*eventCursor); ++eventCursor)
    {
        initialize (event->objectId);
    }
}

void ComponentInitializer::InitializeLights () noexcept
{
    auto initialize = [this] (Emergence::Celerity::UniqueId _lightId)
    {
        auto lightCursor = editLightByLightId.Execute (&_lightId);
        if (auto *light = static_cast<LightComponent *> (*lightCursor))
        {
            light->implementationHandle = INITIALIZE_URHO3D_OBJECT (light, Light);
        }
    };

    for (auto eventCursor = fetchLightAddedNormalEvents.Execute ();
         const auto *event = static_cast<const LightComponentAddedNormalEvent *> (*eventCursor); ++eventCursor)
    {
        initialize (event->lightId);
    }

    for (auto eventCursor = fetchLightAddedCustomEvents.Execute ();
         const auto *event = static_cast<const LightComponentAddedCustomToNormalEvent *> (*eventCursor); ++eventCursor)
    {
        initialize (event->lightId);
    }
}

void ComponentInitializer::InitializeEffects () noexcept
{
    auto initialize = [this] (Emergence::Celerity::UniqueId _modelId)
    {
        auto effectCursor = editParticleEffectByEffectId.Execute (&_modelId);
        if (auto *effect = static_cast<ParticleEffectComponent *> (*effectCursor))
        {
            Urho3D::Node *node = nodeAccessor.StartUsingNode (effect->objectId);
            auto *object = node->CreateComponent<Urho3D::ParticleEmitter> ();
            SyncParticleEffect (effect, object);
            effect->implementationHandle = object;
        }
    };

    for (auto eventCursor = fetchParticleEffectAddedNormalEvents.Execute ();
         const auto *event = static_cast<const ParticleEffectComponentAddedNormalEvent *> (*eventCursor); ++eventCursor)
    {
        initialize (event->effectId);
    }

    for (auto eventCursor = fetchParticleEffectAddedCustomEvents.Execute ();
         const auto *event = static_cast<const ParticleEffectComponentAddedCustomToNormalEvent *> (*eventCursor);
         ++eventCursor)
    {
        initialize (event->effectId);
    }
}

void ComponentInitializer::InitializeModels () noexcept
{
    auto initialize = [this] (Emergence::Celerity::UniqueId _modelId)
    {
        auto modelCursor = editStaticModelByModelId.Execute (&_modelId);
        if (auto *staticModel = static_cast<StaticModelComponent *> (*modelCursor))
        {
            staticModel->implementationHandle = INITIALIZE_URHO3D_OBJECT (staticModel, StaticModel);
        }
    };

    for (auto eventCursor = fetchStaticModelAddedNormalEvents.Execute ();
         const auto *event = static_cast<const StaticModelComponentAddedNormalEvent *> (*eventCursor); ++eventCursor)
    {
        initialize (event->modelId);
    }

    for (auto eventCursor = fetchStaticModelAddedCustomEvents.Execute ();
         const auto *event = static_cast<const StaticModelComponentAddedCustomToNormalEvent *> (*eventCursor);
         ++eventCursor)
    {
        initialize (event->modelId);
    }
}

#undef INITIALIZE_URHO3D_OBJECT

class ComponentSynchronizer : public Emergence::Celerity::TaskExecutorBase<ComponentSynchronizer>
{
public:
    ComponentSynchronizer (Emergence::Celerity::TaskConstructor &_constructor) noexcept;

    void Execute ();

private:
    void SyncCameras () noexcept;

    void SyncLights () noexcept;

    void SyncEffects () noexcept;

    void SyncModels () noexcept;

    Emergence::Celerity::ModifySingletonQuery modifyUrho3D;

    Emergence::Celerity::FetchValueQuery fetchCameraByObjectId;
    Emergence::Celerity::FetchSequenceQuery fetchCameraChangedEvents;

    Emergence::Celerity::FetchValueQuery fetchLightByLightId;
    Emergence::Celerity::FetchSequenceQuery fetchLightChangedEvents;

    Emergence::Celerity::FetchValueQuery fetchParticleEffectByEffectId;
    Emergence::Celerity::FetchSequenceQuery fetchParticleEffectChangedEvents;

    Emergence::Celerity::FetchValueQuery fetchStaticModelByModelId;
    Emergence::Celerity::FetchSequenceQuery fetchStaticModelChangedEvents;
};

ComponentSynchronizer::ComponentSynchronizer (Emergence::Celerity::TaskConstructor &_constructor) noexcept
    : modifyUrho3D (MODIFY_SINGLETON (Urho3DAccessSingleton)),

      fetchCameraByObjectId (FETCH_VALUE_1F (CameraComponent, objectId)),
      fetchCameraChangedEvents (FETCH_SEQUENCE (CameraComponentChangedEvent)),

      fetchLightByLightId (FETCH_VALUE_1F (LightComponent, lightId)),
      fetchLightChangedEvents (FETCH_SEQUENCE (LightComponentChangedEvent)),

      fetchParticleEffectByEffectId (FETCH_VALUE_1F (ParticleEffectComponent, effectId)),
      fetchParticleEffectChangedEvents (FETCH_SEQUENCE (ParticleEffectComponentChangedEvent)),

      fetchStaticModelByModelId (FETCH_VALUE_1F (StaticModelComponent, modelId)),
      fetchStaticModelChangedEvents (FETCH_SEQUENCE (StaticModelComponentChangedEvent))
{
    _constructor.DependOn (TaskNames::INITIALIZE_NEW_COMPONENTS);
}

void ComponentSynchronizer::Execute ()
{
    SyncCameras ();
    SyncLights ();
    SyncEffects ();
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

void ComponentSynchronizer::SyncEffects () noexcept
{
    for (auto eventCursor = fetchParticleEffectChangedEvents.Execute ();
         const auto *event = static_cast<const ParticleEffectComponentChangedEvent *> (*eventCursor); ++eventCursor)
    {
        auto particleEffectCursor = fetchParticleEffectByEffectId.Execute (&event->effectId);
        if (const auto *particleEffect = static_cast<const ParticleEffectComponent *> (*particleEffectCursor))
        {
            SyncParticleEffect (particleEffect,
                                static_cast<Urho3D::ParticleEmitter *> (particleEffect->implementationHandle));
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
    Emergence::Celerity::FetchSequenceQuery fetchParticleEffectRemovedEvents;
    Emergence::Celerity::FetchSequenceQuery fetchStaticModelRemovedEvents;
};

ComponentDeleter::ComponentDeleter (Emergence::Celerity::TaskConstructor &_constructor) noexcept
    : modifyUrho3D (MODIFY_SINGLETON (Urho3DAccessSingleton)),
      modifyRenderScene (MODIFY_SINGLETON (RenderSceneSingleton)),
      nodeAccessor (_constructor),

      fetchCameraRemovedEvents (FETCH_SEQUENCE (CameraComponentRemovedEvent)),
      fetchLightRemovedEvents (FETCH_SEQUENCE (LightComponentRemovedEvent)),
      fetchParticleEffectRemovedEvents (FETCH_SEQUENCE (ParticleEffectComponentRemovedEvent)),
      fetchStaticModelRemovedEvents (FETCH_SEQUENCE (StaticModelComponentRemovedEvent))
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

    for (auto eventCursor = fetchParticleEffectRemovedEvents.Execute ();
         const auto *event = static_cast<const ParticleEffectComponentRemovedEvent *> (*eventCursor); ++eventCursor)
    {
        Urho3D::Node *node = nodeAccessor.UnlinkNode (event->objectId);
        if (node && event->implementationHandle)
        {
            node->RemoveComponent (static_cast<Urho3D::ParticleEmitter *> (event->implementationHandle));
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
    : modifyUrho3D (MODIFY_SINGLETON (Urho3DAccessSingleton)),
      removeUnusedNodes (REMOVE_SIGNAL (Urho3DNodeComponent, usages, 0u))
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

    void UpdateObjectTransform (Emergence::Celerity::UniqueId _objectId) noexcept;

    void UpdateObjectTransform (const Emergence::Celerity::Transform3dComponent *_transform) noexcept;

    void UpdateTransformRecursively (Emergence::Celerity::UniqueId _rootObjectId) noexcept;

    void UpdateTransformRecursively (const Emergence::Celerity::Transform3dComponent *_rootTransform) noexcept;

    void ApplyRenderSceneChanges () noexcept;

    void UpdateUrho3DScene () noexcept;

    Emergence::Celerity::ModifySingletonQuery modifyUrho3D;
    Emergence::Celerity::FetchSingletonQuery fetchTime;
    Emergence::Celerity::FetchSingletonQuery fetchUrho3DScene;
    Emergence::Celerity::FetchSingletonQuery fetchRenderScene;

    Emergence::Celerity::FetchSequenceQuery fetchUrho3DNodeComponentAddedEvents;
    Emergence::Celerity::FetchSequenceQuery fetchTransform3dComponentAddedFixedToNormalEvents;
    Emergence::Celerity::FetchSequenceQuery fetchTransform3dComponentAddedNormalEvents;
    Emergence::Celerity::FetchSequenceQuery fetchTransform3dComponentChangedFixedToNormalEvents;
    Emergence::Celerity::FetchSequenceQuery fetchTransform3dComponentChangedNormalEvents;

    Emergence::Celerity::FetchValueQuery fetchUrho3DNodeById;
    Emergence::Celerity::FetchValueQuery fetchTransformByObjectId;
    Emergence::Celerity::FetchValueQuery fetchTransformByParentObjectId;
    Emergence::Celerity::Transform3dWorldAccessor transformWorldAccessor;

    Emergence::Celerity::FetchSequenceQuery fetchRenderSceneChangedNormalEvents;
    Emergence::Celerity::FetchSequenceQuery fetchRenderSceneChangedCustomEvents;
    Emergence::Celerity::FetchValueQuery fetchCameraByObjectId;
};

SceneUpdater::SceneUpdater (Emergence::Celerity::TaskConstructor &_constructor) noexcept
    : modifyUrho3D (MODIFY_SINGLETON (Urho3DAccessSingleton)),
      fetchTime (FETCH_SINGLETON (Emergence::Celerity::TimeSingleton)),
      fetchUrho3DScene (FETCH_SINGLETON (Urho3DSceneSingleton)),
      fetchRenderScene (FETCH_SINGLETON (RenderSceneSingleton)),

      fetchUrho3DNodeComponentAddedEvents (FETCH_SEQUENCE (Urho3dNodeComponentAddedNormalEvent)),
      fetchTransform3dComponentAddedFixedToNormalEvents (
          FETCH_SEQUENCE (Emergence::Celerity::Transform3dComponentAddedFixedToNormalEvent)),
      fetchTransform3dComponentAddedNormalEvents (
          FETCH_SEQUENCE (Emergence::Celerity::Transform3dComponentAddedNormalEvent)),
      fetchTransform3dComponentChangedFixedToNormalEvents (
          FETCH_SEQUENCE (Emergence::Celerity::Transform3dComponentLocalVisualTransformChangedFixedToNormalEvent)),
      fetchTransform3dComponentChangedNormalEvents (
          FETCH_SEQUENCE (Emergence::Celerity::Transform3dComponentLocalVisualTransformChangedNormalEvent)),

      fetchUrho3DNodeById (FETCH_VALUE_1F (Urho3DNodeComponent, objectId)),
      fetchTransformByObjectId (FETCH_VALUE_1F (Emergence::Celerity::Transform3dComponent, objectId)),
      fetchTransformByParentObjectId (FETCH_VALUE_1F (Emergence::Celerity::Transform3dComponent, parentObjectId)),
      transformWorldAccessor (_constructor),

      fetchRenderSceneChangedNormalEvents (FETCH_SEQUENCE (RenderSceneChangedNormalEvent)),
      fetchRenderSceneChangedCustomEvents (FETCH_SEQUENCE (RenderSceneChangedCustomToNormalEvent)),
      fetchCameraByObjectId (FETCH_VALUE_1F (CameraComponent, objectId))
{
    _constructor.DependOn (Emergence::Celerity::VisualTransformSync::Checkpoint::SYNC_FINISHED);
    _constructor.DependOn (TaskNames::CLEANUP_UNUSED_NODES);
    _constructor.MakeDependencyOf (Checkpoint::RENDER_UPDATE_FINISHED);
    _constructor.MakeDependencyOf (Emergence::Celerity::HierarchyCleanup::Checkpoint::DETACHMENT_DETECTION_STARTED);
}

void SceneUpdater::Execute ()
{
    UpdateTransforms ();
    ApplyRenderSceneChanges ();
    UpdateUrho3DScene ();
}

void SceneUpdater::UpdateTransforms () noexcept
{
    for (auto eventCursor = fetchUrho3DNodeComponentAddedEvents.Execute ();
         const auto *event = static_cast<const Urho3dNodeComponentAddedNormalEvent *> (*eventCursor); ++eventCursor)
    {
        UpdateObjectTransform (event->objectId);
    }

    for (auto eventCursor = fetchTransform3dComponentAddedFixedToNormalEvents.Execute ();
         const auto *event =
             static_cast<const Emergence::Celerity::Transform3dComponentAddedFixedToNormalEvent *> (*eventCursor);
         ++eventCursor)
    {
        UpdateObjectTransform (event->objectId);
    }

    for (auto eventCursor = fetchTransform3dComponentAddedNormalEvents.Execute ();
         const auto *event =
             static_cast<const Emergence::Celerity::Transform3dComponentAddedNormalEvent *> (*eventCursor);
         ++eventCursor)
    {
        UpdateObjectTransform (event->objectId);
    }

    for (auto eventCursor = fetchTransform3dComponentChangedFixedToNormalEvents.Execute ();
         const auto *event = static_cast<
             const Emergence::Celerity::Transform3dComponentLocalVisualTransformChangedFixedToNormalEvent *> (
             *eventCursor);
         ++eventCursor)
    {
        UpdateTransformRecursively (event->objectId);
    }

    for (auto eventCursor = fetchTransform3dComponentChangedNormalEvents.Execute ();
         const auto *event =
             static_cast<const Emergence::Celerity::Transform3dComponentLocalVisualTransformChangedNormalEvent *> (
                 *eventCursor);
         ++eventCursor)
    {
        UpdateTransformRecursively (event->objectId);
    }
}

void SceneUpdater::UpdateObjectTransform (Emergence::Celerity::UniqueId _objectId) noexcept
{
    auto transformCursor = fetchTransformByObjectId.Execute (&_objectId);
    const auto *transform = static_cast<const Emergence::Celerity::Transform3dComponent *> (*transformCursor);

    if (!transform)
    {
        return;
    }

    UpdateObjectTransform (transform);
}

void SceneUpdater::UpdateObjectTransform (const Emergence::Celerity::Transform3dComponent *_transform) noexcept
{
    Emergence::Celerity::UniqueId objectId = _transform->GetObjectId ();
    auto nodeCursor = fetchUrho3DNodeById.Execute (&objectId);
    const auto *node = static_cast<const Urho3DNodeComponent *> (*nodeCursor);

    if (!node)
    {
        return;
    }

    const Emergence::Math::Transform3d &worldTransform = _transform->GetVisualWorldTransform (transformWorldAccessor);
    node->node->SetTransform (
        {worldTransform.translation.x, worldTransform.translation.y, worldTransform.translation.z},
        {worldTransform.rotation.w, worldTransform.rotation.x, worldTransform.rotation.y, worldTransform.rotation.z},
        {worldTransform.scale.x, worldTransform.scale.y, worldTransform.scale.z});
}

void SceneUpdater::UpdateTransformRecursively (Emergence::Celerity::UniqueId _rootObjectId) noexcept
{
    auto transformCursor = fetchTransformByObjectId.Execute (&_rootObjectId);
    const auto *transform = static_cast<const Emergence::Celerity::Transform3dComponent *> (*transformCursor);

    if (!transform)
    {
        return;
    }

    UpdateTransformRecursively (transform);
}

void SceneUpdater::UpdateTransformRecursively (const Emergence::Celerity::Transform3dComponent *_rootTransform) noexcept
{
    UpdateObjectTransform (_rootTransform);
    Emergence::Celerity::UniqueId rootObjectId = _rootTransform->GetObjectId ();

    for (auto cursor = fetchTransformByParentObjectId.Execute (&rootObjectId);
         const auto *transform = static_cast<const Emergence::Celerity::Transform3dComponent *> (*cursor); ++cursor)
    {
        UpdateTransformRecursively (transform);
    }
}

void SceneUpdater::ApplyRenderSceneChanges () noexcept
{
    auto normalEventCursor = fetchRenderSceneChangedNormalEvents.Execute ();
    auto customEventCursor = fetchRenderSceneChangedCustomEvents.Execute ();

    if (!*normalEventCursor && !*customEventCursor)
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
    switch (_light->type)
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

static void SyncParticleEffect (const ParticleEffectComponent *_effect, Urho3D::ParticleEmitter *_urho3DEffect) noexcept
{
    auto *cache = _urho3DEffect->GetContext ()->GetSubsystem<Urho3D::ResourceCache> ();
    if (*_effect->effectName)
    {
        _urho3DEffect->SetEffect (cache->GetResource<Urho3D::ParticleEffect> (*_effect->effectName));
    }
    else
    {
        _urho3DEffect->SetEffect (nullptr);
    }

    _urho3DEffect->SetEmitting (_effect->playing);
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

    const std::size_t materialCount = std::min (_staticModel->materialNames.GetCount (),
                                                static_cast<size_t> (_urho3DStaticModel->GetBatches ().Size ()));

    for (std::size_t index = 0u; index < materialCount; ++index)
    {
        if (*_staticModel->materialNames[index])
        {
            _urho3DStaticModel->SetMaterial (
                static_cast<unsigned int> (index),
                cache->GetResource<Urho3D::Material> (*_staticModel->materialNames[index]));
        }
        else
        {
            _urho3DStaticModel->SetMaterial (static_cast<unsigned int> (index), nullptr);
        }
    }
}

void AddToNormalUpdate (Urho3D::Context *_context, Emergence::Celerity::PipelineBuilder &_pipelineBuilder) noexcept
{
    using namespace Emergence::Memory::Literals;

    _pipelineBuilder.AddTask (TaskNames::ENSURE_SCENE_IS_READY).SetExecutor<SceneInitializer> (_context);

    _pipelineBuilder.AddTask ("Render::RemoveNormalCameras"_us)
        .AS_CASCADE_REMOVER_1F (Emergence::Celerity::Transform3dComponentRemovedNormalEvent, CameraComponent, objectId)
        .DependOn (Checkpoint::RENDER_UPDATE_STARTED)
        // Because Urho3DUpdate is de facto last mechanics in the frame, we take care of hierarchy cleanup here.
        .DependOn (Emergence::Celerity::HierarchyCleanup::Checkpoint::DETACHED_REMOVAL_FINISHED);

    _pipelineBuilder.AddTask ("Render::RemoveFixedCameras"_us)
        .AS_CASCADE_REMOVER_1F (Emergence::Celerity::Transform3dComponentRemovedFixedToNormalEvent, CameraComponent,
                                objectId)
        .DependOn ("Render::RemoveNormalCameras"_us)
        .MakeDependencyOf (TaskNames::INITIALIZE_NEW_COMPONENTS);

    _pipelineBuilder.AddTask ("Render::RemoveNormalLights"_us)
        .AS_CASCADE_REMOVER_1F (Emergence::Celerity::Transform3dComponentRemovedNormalEvent, LightComponent, objectId)
        .DependOn (Checkpoint::RENDER_UPDATE_STARTED)
        .DependOn (Emergence::Celerity::HierarchyCleanup::Checkpoint::DETACHED_REMOVAL_FINISHED);

    _pipelineBuilder.AddTask ("Render::RemoveFixedLights"_us)
        .AS_CASCADE_REMOVER_1F (Emergence::Celerity::Transform3dComponentRemovedFixedToNormalEvent, LightComponent,
                                objectId)
        .DependOn ("Render::RemoveNormalLights"_us)
        .MakeDependencyOf (TaskNames::INITIALIZE_NEW_COMPONENTS);

    _pipelineBuilder.AddTask ("Render::RemoveNormalEffects"_us)
        .AS_CASCADE_REMOVER_1F (Emergence::Celerity::Transform3dComponentRemovedNormalEvent, ParticleEffectComponent,
                                objectId)
        .DependOn (Checkpoint::RENDER_UPDATE_STARTED)
        .DependOn (Emergence::Celerity::HierarchyCleanup::Checkpoint::DETACHED_REMOVAL_FINISHED);

    _pipelineBuilder.AddTask ("Render::RemoveFixedEffects"_us)
        .AS_CASCADE_REMOVER_1F (Emergence::Celerity::Transform3dComponentRemovedFixedToNormalEvent,
                                ParticleEffectComponent, objectId)
        .DependOn ("Render::RemoveNormalEffects"_us)
        .MakeDependencyOf (TaskNames::INITIALIZE_NEW_COMPONENTS);

    _pipelineBuilder.AddTask ("Render::RemoveNormalModels"_us)
        .AS_CASCADE_REMOVER_1F (Emergence::Celerity::Transform3dComponentRemovedNormalEvent, StaticModelComponent,
                                objectId)
        .DependOn (Checkpoint::RENDER_UPDATE_STARTED)
        .DependOn (Emergence::Celerity::HierarchyCleanup::Checkpoint::DETACHED_REMOVAL_FINISHED);

    _pipelineBuilder.AddTask ("Render::RemoveFixedModels"_us)
        .AS_CASCADE_REMOVER_1F (Emergence::Celerity::Transform3dComponentRemovedFixedToNormalEvent,
                                StaticModelComponent, objectId)
        .DependOn ("Render::RemoveNormalModels"_us)
        .MakeDependencyOf (TaskNames::INITIALIZE_NEW_COMPONENTS);

    _pipelineBuilder.AddTask (TaskNames::INITIALIZE_NEW_COMPONENTS).SetExecutor<ComponentInitializer> ();
    _pipelineBuilder.AddTask (TaskNames::APPLY_COMPONENT_CHANGES).SetExecutor<ComponentSynchronizer> ();
    _pipelineBuilder.AddTask (TaskNames::APPLY_COMPONENT_DELETION).SetExecutor<ComponentDeleter> ();
    _pipelineBuilder.AddTask (TaskNames::CLEANUP_UNUSED_NODES).SetExecutor<Urho3DNodeCleaner> ();
    _pipelineBuilder.AddTask (TaskNames::UPDATE_SCENE).SetExecutor<SceneUpdater> ();
}
} // namespace Urho3DUpdate

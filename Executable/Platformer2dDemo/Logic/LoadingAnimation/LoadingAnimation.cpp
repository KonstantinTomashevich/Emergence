#include <limits>

#include <Celerity/Assembly/AssemblyDescriptor.hpp>
#include <Celerity/Assembly/PrototypeComponent.hpp>
#include <Celerity/Model/TimeSingleton.hpp>
#include <Celerity/Model/WorldSingleton.hpp>
#include <Celerity/PipelineBuilderMacros.hpp>
#include <Celerity/Render/2d/Camera2dComponent.hpp>
#include <Celerity/Render/2d/Render2dSingleton.hpp>
#include <Celerity/Render/2d/World2dRenderPass.hpp>
#include <Celerity/Render/Foundation/MaterialInstance.hpp>
#include <Celerity/Render/Foundation/Viewport.hpp>
#include <Celerity/Resource/Object/Messages.hpp>
#include <Celerity/Transform/TransformComponent.hpp>

#include <Configuration/VisibilityMask.hpp>

#include <LoadingAnimation/LoadingAnimation.hpp>
#include <LoadingAnimation/LoadingAnimationSingleton.hpp>

#include <Math/Constants.hpp>

#include <Render/Backend/Configuration.hpp>

namespace LoadingAnimation
{
using namespace Emergence::Memory::Literals;

const Emergence::Memory::UniqueString Checkpoint::STARTED {"LoadingAnimation::Started"};
const Emergence::Memory::UniqueString Checkpoint::FINISHED {"LoadingAnimation::Finished"};

static const Emergence::Memory::UniqueString LOADING_VIEWPORT_NAME {"LoadingAnimationViewport"};
static const Emergence::Memory::UniqueString LOADING_PROTOTYPE_NAME {"RO_LoadingAnimation"};

class Manager final : public Emergence::Celerity::TaskExecutorBase<Manager>
{
public:
    Manager (Emergence::Celerity::TaskConstructor &_constructor) noexcept;

    void Execute () noexcept;

private:
    void RequestDescriptorLoading () noexcept;

    bool IsDescriptorLoaded () noexcept;

    void Instantiate (LoadingAnimationSingleton *_loadingAnimation) noexcept;

    void Destroy (LoadingAnimationSingleton *_loadingAnimation) noexcept;

    Emergence::Celerity::FetchSingletonQuery fetchTime;
    Emergence::Celerity::FetchSingletonQuery fetchWorld;
    Emergence::Celerity::ModifySingletonQuery modifyLoadingAnimation;

    Emergence::Celerity::InsertShortTermQuery insertResourceObjectRequest;
    Emergence::Celerity::FetchValueQuery fetchAssemblyDescriptorById;

    Emergence::Celerity::InsertLongTermQuery insertTransform;
    Emergence::Celerity::InsertLongTermQuery insertCamera;
    Emergence::Celerity::InsertLongTermQuery insertPrototype;
    Emergence::Celerity::InsertLongTermQuery insertViewport;
    Emergence::Celerity::InsertLongTermQuery insertWorldPass;

    Emergence::Celerity::RemoveValueQuery removeViewportByName;
    Emergence::Celerity::RemoveValueQuery removeTransformById;

    Emergence::Celerity::EditValueQuery editUniformVector4fByAssetIdAndName;
};

Manager::Manager (Emergence::Celerity::TaskConstructor &_constructor) noexcept
    : fetchTime (FETCH_SINGLETON (Emergence::Celerity::TimeSingleton)),
      fetchWorld (FETCH_SINGLETON (Emergence::Celerity::WorldSingleton)),
      modifyLoadingAnimation (MODIFY_SINGLETON (LoadingAnimationSingleton)),

      insertResourceObjectRequest (INSERT_SHORT_TERM (Emergence::Celerity::ResourceObjectRequest)),
      fetchAssemblyDescriptorById (FETCH_VALUE_1F (Emergence::Celerity::AssemblyDescriptor, id)),

      insertTransform (INSERT_LONG_TERM (Emergence::Celerity::Transform2dComponent)),
      insertCamera (INSERT_LONG_TERM (Emergence::Celerity::Camera2dComponent)),
      insertPrototype (INSERT_LONG_TERM (Emergence::Celerity::PrototypeComponent)),
      insertViewport (INSERT_LONG_TERM (Emergence::Celerity::Viewport)),
      insertWorldPass (INSERT_LONG_TERM (Emergence::Celerity::World2dRenderPass)),

      removeViewportByName (REMOVE_VALUE_1F (Emergence::Celerity::Viewport, name)),
      removeTransformById (REMOVE_VALUE_1F (Emergence::Celerity::Transform2dComponent, objectId)),

      editUniformVector4fByAssetIdAndName (
          EDIT_VALUE_2F (Emergence::Celerity::UniformVector4fValue, assetId, uniformName))
{
    _constructor.DependOn (Checkpoint::STARTED);
    _constructor.MakeDependencyOf (Checkpoint::FINISHED);
}

void Manager::Execute () noexcept
{
    auto loadingAnimationCursor = modifyLoadingAnimation.Execute ();
    auto *loadingAnimation = static_cast<LoadingAnimationSingleton *> (*loadingAnimationCursor);
    RequestDescriptorLoading ();

    if (loadingAnimation->required != loadingAnimation->instanced)
    {
        if (loadingAnimation->required)
        {
            if (IsDescriptorLoaded ())
            {
                Instantiate (loadingAnimation);
            }
        }
        else
        {
            Destroy (loadingAnimation);
        }
    }

    auto timeCursor = fetchTime.Execute ();
    const auto *time = static_cast<const Emergence::Celerity::TimeSingleton *> (*timeCursor);

    struct
    {
        Emergence::Memory::UniqueString assetId = "MI_CrateLoading"_us;
        Emergence::Memory::UniqueString uniformName = "angle"_us;
    } query;

    if (auto uniformCursor = editUniformVector4fByAssetIdAndName.Execute (&query);
        auto *uniform = static_cast<Emergence::Celerity::UniformVector4fValue *> (*uniformCursor))
    {
        uniform->value.x =
            static_cast<float> (time->realNormalTimeNs % 1000000000u) * 1e-9f * 2.0f * Emergence::Math::PI;
    }
}

void Manager::RequestDescriptorLoading () noexcept
{
    if (!IsDescriptorLoaded ())
    {
        auto cursor = insertResourceObjectRequest.Execute ();
        auto *request = static_cast<Emergence::Celerity::ResourceObjectRequest *> (++cursor);
        request->objects.emplace_back (LOADING_PROTOTYPE_NAME);
    }
}

bool Manager::IsDescriptorLoaded () noexcept
{
    return *fetchAssemblyDescriptorById.Execute (&LOADING_PROTOTYPE_NAME);
}

void Manager::Instantiate (LoadingAnimationSingleton *_loadingAnimation) noexcept
{
    auto worldCursor = fetchWorld.Execute ();
    const auto *world = static_cast<const Emergence::Celerity::WorldSingleton *> (*worldCursor);

    auto transformCursor = insertTransform.Execute ();
    auto cameraCursor = insertCamera.Execute ();
    auto prototypeCursor = insertPrototype.Execute ();

    auto *cameraTransform = static_cast<Emergence::Celerity::Transform2dComponent *> (++transformCursor);
    _loadingAnimation->cameraObjectId = world->GenerateId ();
    cameraTransform->SetObjectId (_loadingAnimation->cameraObjectId);
    cameraTransform->SetVisualLocalTransform ({{0.0f, 0.0f}, 0.0f, {1.0f, 1.0f}});

    auto *camera = static_cast<Emergence::Celerity::Camera2dComponent *> (++cameraCursor);
    camera->objectId = cameraTransform->GetObjectId ();
    camera->halfOrthographicSize = 5.0f;
    camera->visibilityMask = static_cast<std::uint64_t> (VisibilityMask::LOADING_ANIMATION);

    auto viewportCursor = insertViewport.Execute ();
    auto *worldViewport = static_cast<Emergence::Celerity::Viewport *> (++viewportCursor);

    worldViewport->name = LOADING_VIEWPORT_NAME;
    worldViewport->width = Emergence::Render::Backend::GetCurrentConfig ().width;
    worldViewport->height = Emergence::Render::Backend::GetCurrentConfig ().height;
    worldViewport->clearColor = 0x000000FF;
    worldViewport->sortIndex = std::numeric_limits<decltype (worldViewport->sortIndex)>::max ();

    auto worldPassCursor = insertWorldPass.Execute ();
    auto *worldPass = static_cast<Emergence::Celerity::World2dRenderPass *> (++worldPassCursor);

    worldPass->name = worldViewport->name;
    worldPass->cameraObjectId = camera->objectId;

    auto *loadingAnimationTransform = static_cast<Emergence::Celerity::Transform2dComponent *> (++transformCursor);
    _loadingAnimation->sceneObjectId = world->GenerateId ();
    loadingAnimationTransform->SetObjectId (_loadingAnimation->sceneObjectId);

    auto *prototype = static_cast<Emergence::Celerity::PrototypeComponent *> (++prototypeCursor);
    prototype->objectId = _loadingAnimation->sceneObjectId;
    prototype->descriptorId = LOADING_PROTOTYPE_NAME;
    prototype->requestImmediateFixedAssembly = false;
    prototype->requestImmediateNormalAssembly = false;

    _loadingAnimation->instanced = true;
}

void Manager::Destroy (LoadingAnimationSingleton *_loadingAnimation) noexcept
{
    auto removeViewportCursor = removeViewportByName.Execute (&LOADING_VIEWPORT_NAME);
    if (removeViewportCursor.ReadConst ())
    {
        ~removeViewportCursor;
    }

    {
        auto removeCameraTransformCursor = removeTransformById.Execute (&_loadingAnimation->cameraObjectId);
        if (removeCameraTransformCursor.ReadConst ())
        {
            ~removeCameraTransformCursor;
        }
    }

    {
        auto removeSceneTransformCursor = removeTransformById.Execute (&_loadingAnimation->sceneObjectId);
        if (removeSceneTransformCursor.ReadConst ())
        {
            ~removeSceneTransformCursor;
        }
    }

    _loadingAnimation->instanced = false;
}

void AddToNormalUpdate (Emergence::Celerity::PipelineBuilder &_pipelineBuilder) noexcept
{
    _pipelineBuilder.AddCheckpoint (Checkpoint::STARTED);
    _pipelineBuilder.AddCheckpoint (Checkpoint::FINISHED);
    _pipelineBuilder.AddTask (Emergence::Memory::UniqueString {"LoadingAnimationManager"}).SetExecutor<Manager> ();
}
} // namespace LoadingAnimation

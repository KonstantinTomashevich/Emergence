#include <Celerity/Assembly/Assembly.hpp>
#include <Celerity/PipelineBuilderMacros.hpp>
#include <Celerity/Transform/TransformComponent.hpp>
#include <Celerity/Transform/TransformVisualSync.hpp>
#include <Celerity/Transform/TransformWorldAccessor.hpp>

#include <Gameplay/FollowCamera.hpp>
#include <Gameplay/FollowCameraSettingsSingleton.hpp>

#include <Gameplay/ControllableComponent.hpp>

#include <Render/RenderSceneSingleton.hpp>
#include <Render/Urho3DUpdate.hpp>

namespace FollowCamera
{
using namespace Emergence::Memory::Literals;

const Emergence::Memory::UniqueString Checkpoint::STARTED {"FollowCameraStarted"};
const Emergence::Memory::UniqueString Checkpoint::FINISHED {"FollowCameraFinished"};

class CameraUpdater : public Emergence::Celerity::TaskExecutorBase<CameraUpdater>
{
public:
    CameraUpdater (Emergence::Celerity::TaskConstructor &_constructor) noexcept;

    void Execute () noexcept;

private:
    bool FetchControlledUnitTransform (Emergence::Math::Transform3d &_output) noexcept;

    Emergence::Celerity::FetchSingletonQuery fetchRenderScene;
    Emergence::Celerity::FetchSingletonQuery fetchFollowCameraSettings;

    Emergence::Celerity::FetchSignalQuery fetchControlledObject;
    Emergence::Celerity::EditValueQuery editTransformById;
    Emergence::Celerity::Transform3dWorldAccessor transformWorldAccessor;
};

CameraUpdater::CameraUpdater (Emergence::Celerity::TaskConstructor &_constructor) noexcept
    : fetchRenderScene (FETCH_SINGLETON (RenderSceneSingleton)),
      fetchFollowCameraSettings (FETCH_SINGLETON (FollowCameraSettingsSingleton)),

      fetchControlledObject (FETCH_SIGNAL (ControllableComponent, controlledByLocalPlayer, true)),
      editTransformById (EDIT_VALUE_1F (Emergence::Celerity::Transform3dComponent, objectId)),
      transformWorldAccessor (_constructor)
{
    _constructor.DependOn (Checkpoint::STARTED);
    _constructor.DependOn (Emergence::Celerity::Assembly::Checkpoint::FINISHED);
    _constructor.DependOn (Emergence::Celerity::TransformVisualSync::Checkpoint::FINISHED);
    _constructor.MakeDependencyOf (Checkpoint::FINISHED);
    _constructor.MakeDependencyOf (Urho3DUpdate::Checkpoint::STARTED);
}

void CameraUpdater::Execute () noexcept
{
    Emergence::Math::Transform3d unitTransform;
    if (FetchControlledUnitTransform (unitTransform))
    {
        auto renderSceneCursor = fetchRenderScene.Execute ();
        const auto *renderScene = static_cast<const RenderSceneSingleton *> (*renderSceneCursor);

        auto followCameraSettingsCursor = fetchFollowCameraSettings.Execute ();
        const auto *followCameraSettings =
            static_cast<const FollowCameraSettingsSingleton *> (*followCameraSettingsCursor);

        auto cameraTransformCursor = editTransformById.Execute (&renderScene->cameraObjectId);
        if (auto *cameraTransform = static_cast<Emergence::Celerity::Transform3dComponent *> (*cameraTransformCursor))
        {
            // We assume that camera is always a top level citizen.
            EMERGENCE_ASSERT (cameraTransform->GetParentObjectId () == Emergence::Celerity::INVALID_UNIQUE_ID);
            cameraTransform->SetVisualLocalTransform (unitTransform * followCameraSettings->cameraLocalTransform);
        }
    }
}

bool CameraUpdater::FetchControlledUnitTransform (Emergence::Math::Transform3d &_output) noexcept
{
    auto controllableCursor = fetchControlledObject.Execute ();
    if (const auto *controllable = static_cast<const ControllableComponent *> (*controllableCursor))
    {
        auto transformCursor = editTransformById.Execute (&controllable->objectId);
        if (const auto *transform = static_cast<const Emergence::Celerity::Transform3dComponent *> (*transformCursor))
        {
            auto token = editTransformById.AllowUnsafeFetchAccess ();
            _output = transform->GetVisualWorldTransform (transformWorldAccessor);
            return true;
        }
    }

    return false;
}

void AddToNormalUpdate (Emergence::Celerity::PipelineBuilder &_pipelineBuilder) noexcept
{
    _pipelineBuilder.AddCheckpoint (Checkpoint::STARTED);
    _pipelineBuilder.AddCheckpoint (Checkpoint::FINISHED);
    _pipelineBuilder.AddTask ("FollowCamera::Updater"_us).SetExecutor<CameraUpdater> ();
}
} // namespace FollowCamera

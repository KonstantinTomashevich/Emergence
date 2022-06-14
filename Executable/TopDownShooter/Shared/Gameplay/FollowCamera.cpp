#include <Celerity/PipelineBuilderMacros.hpp>

#include <Gameplay/FollowCamera.hpp>
#include <Gameplay/FollowCameraSettingsSingleton.hpp>

#include <Gameplay/UnitComponent.hpp>

#include <Render/RenderSceneSingleton.hpp>

#include <Shared/Checkpoint.hpp>

#include <Transform/Transform3dComponent.hpp>
#include <Transform/Transform3dVisualSync.hpp>
#include <Transform/Transform3dWorldAccessor.hpp>

namespace FollowCamera
{
using namespace Emergence::Memory::Literals;

class CameraUpdater : public Emergence::Celerity::TaskExecutorBase<CameraUpdater>
{
public:
    CameraUpdater (Emergence::Celerity::TaskConstructor &_constructor) noexcept;

    void Execute () noexcept;

private:
    bool FetchControlledUnitTransform (Emergence::Math::Transform3d &_output) noexcept;

    Emergence::Celerity::FetchSingletonQuery fetchRenderScene;
    Emergence::Celerity::FetchSingletonQuery fetchFollowCameraSettings;

    Emergence::Celerity::FetchSignalQuery fetchControlledUnit;
    Emergence::Celerity::EditValueQuery editTransformById;
    Emergence::Transform::Transform3dWorldAccessor transformWorldAccessor;
};

CameraUpdater::CameraUpdater (Emergence::Celerity::TaskConstructor &_constructor) noexcept
    : fetchRenderScene (_constructor.MFetchSingleton (RenderSceneSingleton)),
      fetchFollowCameraSettings (_constructor.MFetchSingleton (FollowCameraSettingsSingleton)),

      fetchControlledUnit (_constructor.MFetchSignal (UnitComponent, controlledByPlayer, true)),
      editTransformById (_constructor.MEditValue1F (Emergence::Transform::Transform3dComponent, objectId)),
      transformWorldAccessor (_constructor)
{
    _constructor.DependOn (Emergence::Transform::VisualSync::Checkpoint::SYNC_FINISHED);
    _constructor.DependOn (Checkpoint::CAMERA_UPDATE_STARTED);
    _constructor.MakeDependencyOf (Checkpoint::CAMERA_UPDATE_FINISHED);
    _constructor.MakeDependencyOf (Checkpoint::RENDER_UPDATE_STARTED);
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
        if (auto *cameraTransform = static_cast<Emergence::Transform::Transform3dComponent *> (*cameraTransformCursor))
        {
            // We assume that camera is always a top level citizen.
            assert (cameraTransform->GetParentObjectId () == Emergence::Celerity::INVALID_UNIQUE_ID);
            cameraTransform->SetVisualLocalTransform (unitTransform * followCameraSettings->cameraLocalTransform);
        }
    }
}

bool CameraUpdater::FetchControlledUnitTransform (Emergence::Math::Transform3d &_output) noexcept
{
    auto unitCursor = fetchControlledUnit.Execute ();
    if (const auto *unit = static_cast<const UnitComponent *> (*unitCursor))
    {
        auto transformCursor = editTransformById.Execute (&unit->objectId);
        if (const auto *transform = static_cast<const Emergence::Transform::Transform3dComponent *> (*transformCursor))
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
    _pipelineBuilder.AddTask ("FollowCamera::Updater"_us).SetExecutor<CameraUpdater> ();
}
} // namespace FollowCamera

#include <Celerity/PipelineBuilderMacros.hpp>
#include <Celerity/Render/Foundation/RenderPipelineFoundation.hpp>
#include <Celerity/Transform/Events.hpp>
#include <Celerity/Transform/TransformComponent.hpp>
#include <Celerity/Transform/TransformHierarchyCleanup.hpp>
#include <Celerity/Transform/TransformVisualSync.hpp>
#include <Celerity/Transform/TransformWorldAccessor.hpp>

#include <Platformer/Camera/Camera.hpp>
#include <Platformer/Camera/CameraContextComponent.hpp>
#include <Platformer/Control/ControllableComponent.hpp>
#include <Platformer/Layer/LayerSetup.hpp>

namespace Camera
{
const Emergence::Memory::UniqueString Checkpoint::STARTED {"Camera::Started"};
const Emergence::Memory::UniqueString Checkpoint::FINISHED {"Camera::Finished"};

class CameraManager final : public Emergence::Celerity::TaskExecutorBase<CameraManager>
{
public:
    CameraManager (Emergence::Celerity::TaskConstructor &_constructor) noexcept;

    void Execute () noexcept;

private:
    Emergence::Celerity::FetchSignalQuery fetchPlayerCameraContext;
    Emergence::Celerity::FetchSignalQuery fetchPlayerControllable;
    Emergence::Celerity::EditValueQuery editTransformById;
    Emergence::Celerity::Transform2dWorldAccessor transformWorldAccessor;
};

CameraManager::CameraManager (Emergence::Celerity::TaskConstructor &_constructor) noexcept
    : fetchPlayerCameraContext (FETCH_SIGNAL (CameraContextComponent, target, CameraTarget::PLAYER)),
      fetchPlayerControllable (FETCH_SIGNAL (ControllableComponent, control, ControlType::PLAYER)),
      editTransformById (EDIT_VALUE_1F (Emergence::Celerity::Transform2dComponent, objectId)),
      transformWorldAccessor (_constructor)
{
    _constructor.DependOn (Emergence::Celerity::TransformHierarchyCleanup::Checkpoint::FINISHED);
    _constructor.DependOn (Emergence::Celerity::TransformVisualSync::Checkpoint::FINISHED);
    _constructor.DependOn (Checkpoint::STARTED);
    _constructor.MakeDependencyOf (Checkpoint::FINISHED);
    _constructor.MakeDependencyOf (Emergence::Celerity::RenderPipelineFoundation::Checkpoint::RENDER_STARTED);
    _constructor.MakeDependencyOf (LayerSetup::Checkpoint::STARTED);
}

void CameraManager::Execute () noexcept
{
    auto playerCameraContextCursor = fetchPlayerCameraContext.Execute ();
    const auto *playerCameraContext = static_cast<const CameraContextComponent *> (*playerCameraContextCursor);

    if (!playerCameraContext)
    {
        return;
    }

    auto playerControllableCursor = fetchPlayerControllable.Execute ();
    const auto *playerControllable = static_cast<const ControllableComponent *> (*playerControllableCursor);

    if (!playerControllable)
    {
        return;
    }

    Emergence::Math::Transform2d playerControllableTransform {Emergence::Math::NoInitializationFlag::Confirm ()};
    {
        auto transformCursor = editTransformById.Execute (&playerControllable->objectId);
        auto *transform = static_cast<Emergence::Celerity::Transform2dComponent *> (*transformCursor);

        if (!transform)
        {
            return;
        }

        auto token = editTransformById.AllowUnsafeFetchAccess ();
        playerControllableTransform = transform->GetVisualWorldTransform (transformWorldAccessor);
    }

    if (auto transformCursor = editTransformById.Execute (&playerCameraContext->objectId);
        auto *transform = static_cast<Emergence::Celerity::Transform2dComponent *> (*transformCursor))
    {
        transform->SetVisualLocalTransform (playerControllableTransform);
    }
}

void AddToNormalUpdate (Emergence::Celerity::PipelineBuilder &_pipelineBuilder) noexcept
{
    using namespace Emergence::Memory::Literals;
    _pipelineBuilder.AddCheckpoint (Checkpoint::STARTED);
    _pipelineBuilder.AddCheckpoint (Checkpoint::FINISHED);

    _pipelineBuilder.AddTask ("Camera::RemoveCameraContext"_us)
        .AS_CASCADE_REMOVER_1F (Emergence::Celerity::TransformNodeCleanupNormalEvent, CameraContextComponent, objectId)
        .DependOn (Emergence::Celerity::TransformHierarchyCleanup::Checkpoint::CLEANUP_STARTED)
        .MakeDependencyOf (Emergence::Celerity::TransformHierarchyCleanup::Checkpoint::FINISHED);

    _pipelineBuilder.AddTask ("Camera::Manager"_us).SetExecutor<CameraManager> ();
}
} // namespace Camera

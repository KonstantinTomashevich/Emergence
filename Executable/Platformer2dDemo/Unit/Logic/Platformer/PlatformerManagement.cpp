#include <Celerity/Input/Input.hpp>
#include <Celerity/Input/InputTriggers.hpp>
#include <Celerity/PipelineBuilderMacros.hpp>
#include <Celerity/Render/2d/Camera2dComponent.hpp>
#include <Celerity/Render/2d/World2dRenderPass.hpp>
#include <Celerity/Render/Foundation/RenderPipelineFoundation.hpp>
#include <Celerity/Render/Foundation/Viewport.hpp>
#include <Celerity/Transform/TransformComponent.hpp>
#include <Celerity/Transform/TransformHierarchyCleanup.hpp>
#include <Celerity/Transform/TransformVisualSync.hpp>
#include <Celerity/UI/UIRenderPass.hpp>
#include <Celerity/WorldSingleton.hpp>

#include <Configuration/VisibilityMask.hpp>

#include <Platformer/Camera/CameraContextComponent.hpp>
#include <Platformer/Input/InputActions.hpp>
#include <Platformer/PlatformerManagement.hpp>
#include <Platformer/PlatformerSingleton.hpp>

#include <Render/Backend/Configuration.hpp>

namespace PlatformerManagement
{
using namespace Emergence::Memory::Literals;

const Emergence::Memory::UniqueString Checkpoint::STARTED {"PlatformerManagement::Started"};
const Emergence::Memory::UniqueString Checkpoint::FINISHED {"PlatformerManagement::Finished"};

class Manager final : public Emergence::Celerity::TaskExecutorBase<Manager>
{
public:
    Manager (Emergence::Celerity::TaskConstructor &_constructor) noexcept;

    void Execute () noexcept;

private:
    void Initialize (PlatformerSingleton *_platformer) noexcept;

    Emergence::Celerity::ModifySingletonQuery modifyPlatformer;
    Emergence::Celerity::FetchSingletonQuery fetchWorld;

    Emergence::Celerity::InsertLongTermQuery insertTransform;
    Emergence::Celerity::InsertLongTermQuery insertCamera;
    Emergence::Celerity::InsertLongTermQuery insertCameraContext;
    Emergence::Celerity::InsertLongTermQuery insertViewport;
    Emergence::Celerity::InsertLongTermQuery insertWorldPass;
    Emergence::Celerity::InsertLongTermQuery insertUIPass;
    Emergence::Celerity::InsertLongTermQuery insertKeyTrigger;
};

Manager::Manager (Emergence::Celerity::TaskConstructor &_constructor) noexcept
    : TaskExecutorBase (_constructor),

      modifyPlatformer (MODIFY_SINGLETON (PlatformerSingleton)),
      fetchWorld (FETCH_SINGLETON (Emergence::Celerity::WorldSingleton)),

      insertTransform (INSERT_LONG_TERM (Emergence::Celerity::Transform2dComponent)),
      insertCamera (INSERT_LONG_TERM (Emergence::Celerity::Camera2dComponent)),
      insertCameraContext (INSERT_LONG_TERM (CameraContextComponent)),
      insertViewport (INSERT_LONG_TERM (Emergence::Celerity::Viewport)),
      insertWorldPass (INSERT_LONG_TERM (Emergence::Celerity::World2dRenderPass)),
      insertUIPass (INSERT_LONG_TERM (Emergence::Celerity::UIRenderPass)),
      insertKeyTrigger (INSERT_LONG_TERM (Emergence::Celerity::KeyTrigger))
{
    _constructor.DependOn (Emergence::Celerity::TransformHierarchyCleanup::Checkpoint::FINISHED);
    _constructor.DependOn (Emergence::Celerity::Input::Checkpoint::ACTION_COMPONENT_READ_ALLOWED);
    _constructor.DependOn (Checkpoint::STARTED);
    _constructor.MakeDependencyOf (Checkpoint::FINISHED);
    _constructor.MakeDependencyOf (Emergence::Celerity::RenderPipelineFoundation::Checkpoint::RENDER_STARTED);
    _constructor.MakeDependencyOf (Emergence::Celerity::TransformVisualSync::Checkpoint::STARTED);
}

void Manager::Execute () noexcept
{
    auto platformerCursor = modifyPlatformer.Execute ();
    auto *platformer = static_cast<PlatformerSingleton *> (*platformerCursor);

    if (platformer->state == PlatformerState::NEEDS_INITIALIZATION)
    {
        Initialize (platformer);
    }
}

void Manager::Initialize (PlatformerSingleton *_platformer) noexcept
{
    // Spawn viewports.

    auto worldCursor = fetchWorld.Execute ();
    const auto *world = static_cast<const Emergence::Celerity::WorldSingleton *> (*worldCursor);

    auto transformCursor = insertTransform.Execute ();
    auto *cameraTransform = static_cast<Emergence::Celerity::Transform2dComponent *> (++transformCursor);
    cameraTransform->SetObjectId (world->GenerateId ());
    cameraTransform->SetVisualLocalTransform ({{0.0f, 0.0f}, 0.0f, {1.0f, 1.0f}});

    auto cameraCursor = insertCamera.Execute ();
    auto *camera = static_cast<Emergence::Celerity::Camera2dComponent *> (++cameraCursor);
    camera->objectId = cameraTransform->GetObjectId ();
    camera->halfOrthographicSize = 3.75f;
    camera->visibilityMask = static_cast<std::uint64_t> (VisibilityMask::GAME_SCENE);

    auto cameraContextCursor = insertCameraContext.Execute ();
    auto *cameraContext = static_cast<CameraContextComponent *> (++cameraContextCursor);
    cameraContext->objectId = cameraTransform->GetObjectId ();
    cameraContext->target = CameraTarget::PLAYER;

    auto viewportCursor = insertViewport.Execute ();
    auto *worldViewport = static_cast<Emergence::Celerity::Viewport *> (++viewportCursor);

    worldViewport->name = "PlatformerScene"_us;
    worldViewport->width = Emergence::Render::Backend::GetCurrentConfig ().width;
    worldViewport->height = Emergence::Render::Backend::GetCurrentConfig ().height;
    worldViewport->clearColor = 0xC3FBFFFF;
    worldViewport->sortIndex = 0u;

    auto worldPassCursor = insertWorldPass.Execute ();
    auto *worldPass = static_cast<Emergence::Celerity::World2dRenderPass *> (++worldPassCursor);
    worldPass->name = worldViewport->name;
    worldPass->cameraObjectId = camera->objectId;

    auto *uiViewport = static_cast<Emergence::Celerity::Viewport *> (++viewportCursor);
    uiViewport->name = "PlatformerUI"_us;
    uiViewport->width = Emergence::Render::Backend::GetCurrentConfig ().width;
    uiViewport->height = Emergence::Render::Backend::GetCurrentConfig ().height;
    uiViewport->clearColor = 0x00000000;
    uiViewport->sortIndex = 1u;

    auto uiPassCursor = insertUIPass.Execute ();
    auto *uiPass = static_cast<Emergence::Celerity::UIRenderPass *> (++uiPassCursor);
    uiPass->name = uiViewport->name;
    uiPass->defaultStyleId = "Default"_us;

    // Spawn input triggers.

    auto keyTriggerCursor = insertKeyTrigger.Execute ();
    auto *upTrigger = static_cast<Emergence::Celerity::KeyTrigger *> (++keyTriggerCursor);
    upTrigger->actionToSend = PlatformerInputActions::DIRECTED_MOVEMENT;
    upTrigger->actionToSend.real[0u] = 0.0f;
    upTrigger->actionToSend.real[1u] = 1.0f;
    // TODO: Normally, we need to get this codes from settings, not just hardcode SDL values.
    upTrigger->triggerCode = 26;
    upTrigger->dispatchType = Emergence::Celerity::InputActionDispatchType::FIXED_PERSISTENT;

    auto *downTrigger = static_cast<Emergence::Celerity::KeyTrigger *> (++keyTriggerCursor);
    downTrigger->actionToSend = PlatformerInputActions::DIRECTED_MOVEMENT;
    downTrigger->actionToSend.real[0u] = 0.0f;
    downTrigger->actionToSend.real[1u] = -1.0f;
    downTrigger->triggerCode = 22;
    downTrigger->dispatchType = Emergence::Celerity::InputActionDispatchType::FIXED_PERSISTENT;

    auto *leftTrigger = static_cast<Emergence::Celerity::KeyTrigger *> (++keyTriggerCursor);
    leftTrigger->actionToSend = PlatformerInputActions::DIRECTED_MOVEMENT;
    leftTrigger->actionToSend.real[0u] = -1.0f;
    leftTrigger->actionToSend.real[1u] = 0.0f;
    leftTrigger->triggerCode = 4;
    leftTrigger->dispatchType = Emergence::Celerity::InputActionDispatchType::FIXED_PERSISTENT;

    auto *rightTrigger = static_cast<Emergence::Celerity::KeyTrigger *> (++keyTriggerCursor);
    rightTrigger->actionToSend = PlatformerInputActions::DIRECTED_MOVEMENT;
    rightTrigger->actionToSend.real[0u] = 1.0f;
    rightTrigger->actionToSend.real[1u] = 0.0f;
    rightTrigger->triggerCode = 7;
    rightTrigger->dispatchType = Emergence::Celerity::InputActionDispatchType::FIXED_PERSISTENT;

    auto *rollTrigger = static_cast<Emergence::Celerity::KeyTrigger *> (++keyTriggerCursor);
    rollTrigger->actionToSend = PlatformerInputActions::MOVEMENT_ROLL;
    rollTrigger->triggerCode = 44;
    rollTrigger->triggerType = Emergence::Celerity::KeyTriggerType::ON_STATE_CHANGED;
    rollTrigger->dispatchType = Emergence::Celerity::InputActionDispatchType::FIXED_PERSISTENT;

    _platformer->state = PlatformerState::GAME;
}

Platformer2dDemoLogicApi void AddToNormalUpdate (Emergence::Celerity::PipelineBuilder &_pipelineBuilder) noexcept
{
    _pipelineBuilder.AddCheckpoint (Checkpoint::STARTED);
    _pipelineBuilder.AddCheckpoint (Checkpoint::FINISHED);
    _pipelineBuilder.AddTask ("PlatformerManager"_us).SetExecutor<Manager> ();
}
} // namespace PlatformerManagement

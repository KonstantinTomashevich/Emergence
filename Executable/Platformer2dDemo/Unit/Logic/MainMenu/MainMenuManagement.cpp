#include <Celerity/Input/Input.hpp>
#include <Celerity/Input/InputActionComponent.hpp>
#include <Celerity/Input/InputSubscriptionComponent.hpp>
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

#include <GameCore/GameStateSingleton.hpp>
#include <GameCore/LevelsConfigurationSingleton.hpp>

#include <Render/Backend/Configuration.hpp>

#include <MainMenu/InputActions.hpp>
#include <MainMenu/MainMenuManagement.hpp>
#include <MainMenu/MainMenuSingleton.hpp>

namespace MainMenuManagement
{
using namespace Emergence::Memory::Literals;

const Emergence::Memory::UniqueString Checkpoint::STARTED {"MainMenuManagement::Started"};
const Emergence::Memory::UniqueString Checkpoint::FINISHED {"MainMenuManagement::Finished"};

class Manager final : public Emergence::Celerity::TaskExecutorBase<Manager>
{
public:
    Manager (Emergence::Celerity::TaskConstructor &_constructor) noexcept;

    void Execute () noexcept;

private:
    void Initialize (MainMenuSingleton *_mainMenu);

    void ExecuteForMainMenuState (MainMenuSingleton *_mainMenu);

    Emergence::Celerity::ModifySingletonQuery modifyMainMenu;
    Emergence::Celerity::ModifySingletonQuery modifyGameState;
    Emergence::Celerity::FetchSingletonQuery fetchWorld;
    Emergence::Celerity::FetchSingletonQuery fetchLevelsConfiguration;

    Emergence::Celerity::FetchValueQuery fetchInputActionByObjectId;

    Emergence::Celerity::InsertLongTermQuery insertTransform;
    Emergence::Celerity::InsertLongTermQuery insertCamera;
    Emergence::Celerity::InsertLongTermQuery insertViewport;
    Emergence::Celerity::InsertLongTermQuery insertWorldPass;
    Emergence::Celerity::InsertLongTermQuery insertUIPass;
    Emergence::Celerity::InsertLongTermQuery insertInputSubscription;
};

Manager::Manager (Emergence::Celerity::TaskConstructor &_constructor) noexcept
    : TaskExecutorBase (_constructor),

      modifyMainMenu (MODIFY_SINGLETON (MainMenuSingleton)),
      modifyGameState (MODIFY_SINGLETON (GameStateSingleton)),
      fetchWorld (FETCH_SINGLETON (Emergence::Celerity::WorldSingleton)),
      fetchLevelsConfiguration (FETCH_SINGLETON (LevelsConfigurationSingleton)),

      fetchInputActionByObjectId (FETCH_VALUE_1F (Emergence::Celerity::InputActionComponent, objectId)),

      insertTransform (INSERT_LONG_TERM (Emergence::Celerity::Transform2dComponent)),
      insertCamera (INSERT_LONG_TERM (Emergence::Celerity::Camera2dComponent)),
      insertViewport (INSERT_LONG_TERM (Emergence::Celerity::Viewport)),
      insertWorldPass (INSERT_LONG_TERM (Emergence::Celerity::World2dRenderPass)),
      insertUIPass (INSERT_LONG_TERM (Emergence::Celerity::UIRenderPass)),
      insertInputSubscription (INSERT_LONG_TERM (Emergence::Celerity::InputSubscriptionComponent))
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
    auto mainMenuCursor = modifyMainMenu.Execute ();
    auto *mainMenu = static_cast<MainMenuSingleton *> (*mainMenuCursor);

    if (mainMenu->state == MainMenuState::NEEDS_INITIALIZATION)
    {
        Initialize (mainMenu);
    }

    switch (mainMenu->state)
    {
    case MainMenuState::NEEDS_INITIALIZATION:
        EMERGENCE_ASSERT (false);
        break;

    case MainMenuState::MAIN_MENU:
        ExecuteForMainMenuState (mainMenu);
        break;
    }
}

void Manager::Initialize (MainMenuSingleton *_mainMenu)
{
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

    auto viewportCursor = insertViewport.Execute ();
    auto *worldViewport = static_cast<Emergence::Celerity::Viewport *> (++viewportCursor);

    worldViewport->name = "MainMenuScene"_us;
    worldViewport->width = Emergence::Render::Backend::GetCurrentConfig ().width;
    worldViewport->height = Emergence::Render::Backend::GetCurrentConfig ().height;
    worldViewport->clearColor = 0x000000FF;
    worldViewport->sortIndex = 0u;

    auto worldPassCursor = insertWorldPass.Execute ();
    auto *worldPass = static_cast<Emergence::Celerity::World2dRenderPass *> (++worldPassCursor);
    worldPass->name = worldViewport->name;
    worldPass->cameraObjectId = camera->objectId;

    auto *uiViewport = static_cast<Emergence::Celerity::Viewport *> (++viewportCursor);
    uiViewport->name = "MainMenuUI"_us;
    uiViewport->width = Emergence::Render::Backend::GetCurrentConfig ().width;
    uiViewport->height = Emergence::Render::Backend::GetCurrentConfig ().height;
    uiViewport->clearColor = 0x00000000;
    uiViewport->sortIndex = 1u;

    auto uiPassCursor = insertUIPass.Execute ();
    auto *uiPass = static_cast<Emergence::Celerity::UIRenderPass *> (++uiPassCursor);
    uiPass->name = uiViewport->name;
    uiPass->defaultStyleId = "Default"_us;

    auto inputSubscriptionCursor = insertInputSubscription.Execute ();
    auto *subscription = static_cast<Emergence::Celerity::InputSubscriptionComponent *> (++inputSubscriptionCursor);
    subscription->objectId = world->GenerateId ();
    subscription->group = MainMenuInputActions::GetMainMenuUIGroup ();
    _mainMenu->uiListenerObjectId = subscription->objectId;

    _mainMenu->state = MainMenuState::MAIN_MENU;
}

void Manager::ExecuteForMainMenuState (MainMenuSingleton *_mainMenu)
{
    auto gameStateCursor = modifyGameState.Execute ();
    auto *gameState = static_cast<GameStateSingleton *> (*gameStateCursor);

    for (auto actionCursor = fetchInputActionByObjectId.Execute (&_mainMenu->uiListenerObjectId);
         const auto *action = static_cast<const Emergence::Celerity::InputActionComponent *> (*actionCursor);
         ++actionCursor)
    {
        if (action->action.id == MainMenuInputActions::GetStartTutorialAction ().id)
        {
            auto levelsConfigurationCursor = fetchLevelsConfiguration.Execute ();
            const auto *levelsConfiguration =
                static_cast<const LevelsConfigurationSingleton *> (*levelsConfigurationCursor);

            gameState->request.state = GameState::PLATFORMER_LOADING;
            gameState->request.levelName = levelsConfiguration->tutorialLevelName;
            break;
        }

        if (action->action.id == MainMenuInputActions::GetStartCampaignAction ().id)
        {
            auto levelsConfigurationCursor = fetchLevelsConfiguration.Execute ();
            const auto *levelsConfiguration =
                static_cast<const LevelsConfigurationSingleton *> (*levelsConfigurationCursor);
            EMERGENCE_ASSERT (levelsConfiguration->campaignLevelCount > 0u);

            gameState->request.state = GameState::PLATFORMER_LOADING;
            gameState->request.levelName = Emergence::Memory::UniqueString {
                EMERGENCE_BUILD_STRING (levelsConfiguration->campaignLevelPrefix, "0")};

            break;
        }

        if (action->action.id == MainMenuInputActions::GetQuitAction ().id)
        {
            gameState->request.state = GameState::TERMINATED;
            break;
        }
    }
}

void AddToNormalUpdate (Emergence::Celerity::PipelineBuilder &_pipelineBuilder) noexcept
{
    _pipelineBuilder.AddCheckpoint (Checkpoint::STARTED);
    _pipelineBuilder.AddCheckpoint (Checkpoint::FINISHED);
    _pipelineBuilder.AddTask ("MainMenuManager"_us).SetExecutor<Manager> ();
}
} // namespace MainMenuManagement

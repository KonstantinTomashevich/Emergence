#include <Celerity/Input/Input.hpp>
#include <Celerity/Input/InputActionComponent.hpp>
#include <Celerity/Input/InputSubscriptionComponent.hpp>
#include <Celerity/PipelineBuilderMacros.hpp>
#include <Celerity/WorldSingleton.hpp>

#include <GameCore/GameStateSingleton.hpp>
#include <GameCore/LevelsConfigurationSingleton.hpp>

#include <MainMenu/InputActions.hpp>
#include <MainMenu/MainMenuInputResponse.hpp>
#include <MainMenu/MainMenuSingleton.hpp>

namespace MainMenuInputResponse
{
class Controller final : public Emergence::Celerity::TaskExecutorBase<Controller>
{
public:
    Controller (Emergence::Celerity::TaskConstructor &_constructor) noexcept;

    void Execute () noexcept;

private:
    Emergence::Celerity::FetchSingletonQuery fetchWorld;
    Emergence::Celerity::ModifySingletonQuery modifyMainMenu;
    Emergence::Celerity::FetchSingletonQuery fetchLevelsConfiguration;

    Emergence::Celerity::InsertLongTermQuery insertInputSubscription;
    Emergence::Celerity::ModifySingletonQuery modifyGameState;
    Emergence::Celerity::FetchValueQuery fetchInputActionByObjectId;
};

Controller::Controller (Emergence::Celerity::TaskConstructor &_constructor) noexcept
    : TaskExecutorBase (_constructor),

      fetchWorld (FETCH_SINGLETON (Emergence::Celerity::WorldSingleton)),
      modifyMainMenu (MODIFY_SINGLETON (MainMenuSingleton)),
      fetchLevelsConfiguration (FETCH_SINGLETON (LevelsConfigurationSingleton)),

      insertInputSubscription (INSERT_LONG_TERM (Emergence::Celerity::InputSubscriptionComponent)),
      modifyGameState (MODIFY_SINGLETON (GameStateSingleton)),
      fetchInputActionByObjectId (FETCH_VALUE_1F (Emergence::Celerity::InputActionComponent, objectId))
{
    _constructor.DependOn (Emergence::Celerity::Input::Checkpoint::ACTION_COMPONENT_READ_ALLOWED);
}

void Controller::Execute () noexcept
{
    auto mainMenuCursor = modifyMainMenu.Execute ();
    auto *mainMenu = static_cast<MainMenuSingleton *> (*mainMenuCursor);

    if (mainMenu->uiListenerObjectId == Emergence::Celerity::INVALID_UNIQUE_ID)
    {
        auto worldCursor = fetchWorld.Execute ();
        const auto *world = static_cast<const Emergence::Celerity::WorldSingleton *> (*worldCursor);

        auto inputSubscriptionCursor = insertInputSubscription.Execute ();
        auto *subscription = static_cast<Emergence::Celerity::InputSubscriptionComponent *> (++inputSubscriptionCursor);
        subscription->objectId = world->GenerateId ();
        subscription->group = MainMenuInputActions::GetMainMenuUIGroup ();
        mainMenu->uiListenerObjectId = subscription->objectId;
    }

    auto gameStateCursor = modifyGameState.Execute ();
    auto *gameState = static_cast<GameStateSingleton *> (*gameStateCursor);

    for (auto actionCursor = fetchInputActionByObjectId.Execute (&mainMenu->uiListenerObjectId);
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
    _pipelineBuilder.AddTask (Emergence::Memory::UniqueString {"MainMenuController"}).SetExecutor<Controller> ();
}
} // namespace MainMenuInputResponse

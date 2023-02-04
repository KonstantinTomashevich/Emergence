#include <Celerity/Input/Input.hpp>
#include <Celerity/Input/InputActionComponent.hpp>
#include <Celerity/PipelineBuilderMacros.hpp>

#include <Configuration/WorldStates.hpp>

#include <MainMenu/InputActions.hpp>
#include <MainMenu/MainMenuInputResponse.hpp>
#include <MainMenu/MainMenuSingleton.hpp>

#include <Root/LevelSelectionSingleton.hpp>
#include <Root/LevelsConfigurationSingleton.hpp>

namespace MainMenuInputResponse
{
class Controller final : public Emergence::Celerity::TaskExecutorBase<Controller>
{
public:
    Controller (Emergence::Celerity::TaskConstructor &_constructor,
                const WorldStateRedirectionHandle &_redirectionHandle) noexcept;

    void Execute () noexcept;

private:
    Emergence::Celerity::FetchSingletonQuery fetchMainMenu;
    Emergence::Celerity::FetchSingletonQuery fetchLevelsConfiguration;
    Emergence::Celerity::ModifySingletonQuery modifyLevelSelection;
    Emergence::Celerity::FetchValueQuery fetchInputActionByObjectId;
    WorldStateRedirectionHandle redirectionHandle;
};

Controller::Controller (Emergence::Celerity::TaskConstructor &_constructor,
                        const WorldStateRedirectionHandle &_redirectionHandle) noexcept
    : fetchMainMenu (FETCH_SINGLETON (MainMenuSingleton)),
      fetchLevelsConfiguration (FETCH_SINGLETON (LevelsConfigurationSingleton)),
      modifyLevelSelection (MODIFY_SINGLETON (LevelSelectionSingleton)),
      fetchInputActionByObjectId (FETCH_VALUE_1F (Emergence::Celerity::InputActionComponent, objectId)),
      redirectionHandle (_redirectionHandle)
{
    _constructor.DependOn (Emergence::Celerity::Input::Checkpoint::ACTION_COMPONENT_READ_ALLOWED);
}

void Controller::Execute () noexcept
{
    auto mainMenuCursor = fetchMainMenu.Execute ();
    const auto *mainMenu = static_cast<const MainMenuSingleton *> (*mainMenuCursor);

    for (auto actionCursor = fetchInputActionByObjectId.Execute (&mainMenu->uiListenerObjectId);
         const auto *action = static_cast<const Emergence::Celerity::InputActionComponent *> (*actionCursor);
         ++actionCursor)
    {
        if (action->action.id == MainMenuInputActions::GetStartTutorialAction ().id)
        {
            auto levelsConfigurationCursor = fetchLevelsConfiguration.Execute ();
            const auto *levelsConfiguration =
                static_cast<const LevelsConfigurationSingleton *> (*levelsConfigurationCursor);

            auto levelSelectionCursor = modifyLevelSelection.Execute ();
            auto *levelSelection = static_cast<LevelSelectionSingleton *> (*levelSelectionCursor);
            levelSelection->selectedLevelName = levelsConfiguration->tutorialLevelName;

            redirectionHandle.RequestRedirect (WorldStates::GAME_LEVEL);
            break;
        }

        if (action->action.id == MainMenuInputActions::GetStartCampaignAction ().id)
        {
            auto levelsConfigurationCursor = fetchLevelsConfiguration.Execute ();
            const auto *levelsConfiguration =
                static_cast<const LevelsConfigurationSingleton *> (*levelsConfigurationCursor);
            EMERGENCE_ASSERT (levelsConfiguration->campaignLevelCount > 0u);

            auto levelSelectionCursor = modifyLevelSelection.Execute ();
            auto *levelSelection = static_cast<LevelSelectionSingleton *> (*levelSelectionCursor);

            levelSelection->selectedLevelName = Emergence::Memory::UniqueString {
                EMERGENCE_BUILD_STRING (levelsConfiguration->campaignLevelPrefix, "0")};
            redirectionHandle.RequestRedirect (WorldStates::GAME_LEVEL);
            break;
        }

        if (action->action.id == MainMenuInputActions::GetQuitAction ().id)
        {
            redirectionHandle.RequestRedirect (GameState::TERMINATION_REDIRECT);
            break;
        }
    }
}

void AddToNormalUpdate (Emergence::Celerity::PipelineBuilder &_pipelineBuilder,
                        const WorldStateRedirectionHandle &_redirectionHandle) noexcept
{
    _pipelineBuilder.AddTask (Emergence::Memory::UniqueString {"MainMenuController"})
        .SetExecutor<Controller> (_redirectionHandle);
}
} // namespace MainMenuInputResponse

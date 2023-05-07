#include <MainMenu/InputActions.hpp>

using namespace Emergence::Memory::Literals;

Emergence::Memory::UniqueString MainMenuInputActions::GetMainMenuUIGroup () noexcept
{
    static const Emergence::Memory::UniqueString group {"MainMenuUI"};
    return group;
}

const Emergence::Celerity::InputAction &MainMenuInputActions::GetStartTutorialAction () noexcept
{
    static const Emergence::Celerity::InputAction action {GetMainMenuUIGroup (), "StartTutorial"_us};
    return action;
}

const Emergence::Celerity::InputAction &MainMenuInputActions::GetStartCampaignAction () noexcept
{
    static const Emergence::Celerity::InputAction action {GetMainMenuUIGroup (), "StartCampaign"_us};
    return action;
}

const Emergence::Celerity::InputAction &MainMenuInputActions::GetQuitAction () noexcept
{
    static const Emergence::Celerity::InputAction action {GetMainMenuUIGroup (), "Quit"_us};
    return action;
}

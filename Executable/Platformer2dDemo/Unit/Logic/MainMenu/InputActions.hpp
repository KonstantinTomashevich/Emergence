#pragma once

#include <Platformer2dDemoLogicApi.hpp>

#include <Celerity/Input/InputAction.hpp>

struct Platformer2dDemoLogicApi MainMenuInputActions final
{
    MainMenuInputActions () = delete;

    static Emergence::Memory::UniqueString GetMainMenuUIGroup () noexcept;

    static const Emergence::Celerity::InputAction &GetStartTutorialAction () noexcept;

    static const Emergence::Celerity::InputAction &GetStartCampaignAction () noexcept;

    static const Emergence::Celerity::InputAction &GetQuitAction () noexcept;
};

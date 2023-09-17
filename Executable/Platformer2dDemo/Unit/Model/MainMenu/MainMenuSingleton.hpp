#pragma once

#include <Platformer2dDemoModelApi.hpp>

#include <Celerity/Standard/UniqueId.hpp>

#include <StandardLayout/Mapping.hpp>

enum class MainMenuState : std::uint8_t
{
    NEEDS_INITIALIZATION = 0u,
    MAIN_MENU,
};

struct Platformer2dDemoModelApi MainMenuSingleton final
{
    MainMenuState state = MainMenuState::NEEDS_INITIALIZATION;
    Emergence::Celerity::UniqueId uiListenerObjectId = Emergence::Celerity::INVALID_UNIQUE_ID;

    struct Platformer2dDemoModelApi Reflection final
    {
        Emergence::StandardLayout::FieldId state;
        Emergence::StandardLayout::FieldId uiListenerObjectId;
        Emergence::StandardLayout::Mapping mapping;
    };

    static const Reflection &Reflect () noexcept;
};

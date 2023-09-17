#pragma once

#include <Platformer2dDemoModelApi.hpp>

#include <StandardLayout/Mapping.hpp>

enum class GameState : std::uint8_t
{
    NONE = 0u,
    MAIN_MENU_LOADING,
    MAIN_MENU,
    PLATFORMER_LOADING,
    PLATFORMER_GAME,
    TERMINATED,
};

struct Platformer2dDemoModelApi GameStateRequest final
{
    GameState state = GameState::NONE;

    union
    {
        Emergence::Memory::UniqueString levelName {};
    };

    struct Platformer2dDemoModelApi Reflection final
    {
        Emergence::StandardLayout::FieldId state;
        Emergence::StandardLayout::FieldId levelName;
        Emergence::StandardLayout::Mapping mapping;
    };

    static const Reflection &Reflect () noexcept;
};

struct Platformer2dDemoModelApi GameStateSingleton final
{
    GameState state = GameState::NONE;
    GameStateRequest request;
    GameStateRequest lastRequest;

    struct Platformer2dDemoModelApi Reflection final
    {
        Emergence::StandardLayout::FieldId state;
        Emergence::StandardLayout::FieldId request;
        Emergence::StandardLayout::FieldId lastRequest;
        Emergence::StandardLayout::Mapping mapping;
    };

    static const Reflection &Reflect () noexcept;
};

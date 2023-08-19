#pragma once

#include <Platformer2dDemoModelApi.hpp>

#include <StandardLayout/Mapping.hpp>

enum class PlatformerState : std::uint8_t
{
    NEEDS_INITIALIZATION = 0u,
    GAME,
};

struct Platformer2dDemoModelApi PlatformerSingleton final
{
    PlatformerState state = PlatformerState::NEEDS_INITIALIZATION;

    struct Platformer2dDemoModelApi Reflection final
    {
        Emergence::StandardLayout::FieldId state;
        Emergence::StandardLayout::Mapping mapping;
    };

    static const Reflection &Reflect () noexcept;
};

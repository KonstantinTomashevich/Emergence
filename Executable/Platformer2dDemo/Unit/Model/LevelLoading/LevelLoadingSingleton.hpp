#pragma once

#include <Platformer2dDemoModelApi.hpp>

#include <Celerity/Standard/UniqueId.hpp>

#include <StandardLayout/Mapping.hpp>

enum class LevelLoadingState : std::uint8_t
{
    NONE,
    LOADING_DESCRIPTOR,
    ASSEMBLING,
    DONE,
};

struct Platformer2dDemoModelApi LevelLoadingSingleton final
{
    Emergence::Memory::UniqueString levelName;
    LevelLoadingState state = LevelLoadingState::NONE;

    struct Platformer2dDemoModelApi Reflection final
    {
        Emergence::StandardLayout::FieldId levelName;
        Emergence::StandardLayout::FieldId state;
        Emergence::StandardLayout::Mapping mapping;
    };

    static const Reflection &Reflect () noexcept;
};

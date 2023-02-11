#pragma once

#include <Celerity/Standard/UniqueId.hpp>

#include <StandardLayout/Mapping.hpp>

enum class LevelLoadingState : uint8_t
{
    NONE,
    LOADING_DESCRIPTOR,
    ASSEMBLING,
    DONE,
};

struct LevelLoadingSingleton final
{
    Emergence::Memory::UniqueString levelName;
    LevelLoadingState state = LevelLoadingState::NONE;

    struct Reflection final
    {
        Emergence::StandardLayout::FieldId levelName;
        Emergence::StandardLayout::FieldId state;
        Emergence::StandardLayout::Mapping mapping;
    };

    static const Reflection &Reflect () noexcept;
};

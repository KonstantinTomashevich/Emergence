#pragma once

#include <Container/String.hpp>

#include <StandardLayout/Mapping.hpp>

struct LevelGenerationRequest final
{
    [[maybe_unused]] std::uintptr_t alignmentFixer = 0u;

    struct Reflection final
    {
        Emergence::StandardLayout::Mapping mapping;
    };

    static const Reflection &Reflect () noexcept;
};

struct LevelGenerationFinishedResponse final
{
    [[maybe_unused]] std::uintptr_t alignmentFixer = 0u;

    struct Reflection final
    {
        Emergence::StandardLayout::Mapping mapping;
    };

    static const Reflection &Reflect () noexcept;
};

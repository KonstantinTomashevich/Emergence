#pragma once

#include <Container/String.hpp>

#include <StandardLayout/Mapping.hpp>

struct alignas (uintptr_t) LevelGenerationRequest final
{
    struct Reflection final
    {
        Emergence::StandardLayout::Mapping mapping;
    };

    static const Reflection &Reflect () noexcept;
};

struct alignas (uintptr_t) LevelGenerationFinishedResponse final
{
    struct Reflection final
    {
        Emergence::StandardLayout::Mapping mapping;
    };

    static const Reflection &Reflect () noexcept;
};

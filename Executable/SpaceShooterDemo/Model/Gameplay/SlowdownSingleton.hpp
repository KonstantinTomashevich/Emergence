#pragma once

#include <StandardLayout/Mapping.hpp>

/// \details Slowdown is singleton, because it is global mechanic, that can only be used by player.
struct SlowdownSingleton final
{
    uint64_t durationNs = 1250000000u; // 1.25 seconds.

    uint64_t endTimeNs = 0u;

    float strength = 0.75f;

    struct Reflection
    {
        Emergence::StandardLayout::FieldId durationNs;
        Emergence::StandardLayout::FieldId endTimeNs;
        Emergence::StandardLayout::FieldId strength;
        Emergence::StandardLayout::Mapping mapping;
    };

    static const Reflection &Reflect () noexcept;
};

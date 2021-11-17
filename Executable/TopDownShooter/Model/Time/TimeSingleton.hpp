#pragma once

#include <cstdint>

#include <StandardLayout/Mapping.hpp>

struct TimeSingleton final
{
    std::uint64_t fixedFrameIndex;
    float fixedTimeS;
    float fixedDurationS;

    float normalTimeS;
    float normalDurationS;

    struct Reflection final
    {
        Emergence::StandardLayout::FieldId fixedFrameIndex;
        Emergence::StandardLayout::FieldId fixedTimeS;
        Emergence::StandardLayout::FieldId fixedDurationS;
        Emergence::StandardLayout::FieldId normalTimeS;
        Emergence::StandardLayout::FieldId normalDurationS;
        Emergence::StandardLayout::Mapping mapping;
    };

    static const Reflection &Reflect () noexcept;
};

static_assert (std::is_trivial_v<TimeSingleton>);

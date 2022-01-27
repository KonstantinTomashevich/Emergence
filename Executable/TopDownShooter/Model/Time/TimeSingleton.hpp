#pragma once

#include <cstdint>

#include <StandardLayout/Mapping.hpp>

#include <Utility/RunningAverage.hpp>

/// \note For small periods of time floats are the most suitable format, therefore we them use for frame durations.
///       But for long periods of time (more than several hours) they may become a bit unstable, therefore we
///       use 64-bit microsecond counters;
struct TimeSingleton final
{
    static constexpr std::size_t MAXIMUM_TARGET_FIXED_DURATIONS = 3u;

    uint64_t fixedTimeUs = 0u;

    /// Will be selected automatically from ::targetFixedFrameDurationsS.
    float fixedDurationS = 0.0f;
    Emergence::Container::InplaceVector<float, MAXIMUM_TARGET_FIXED_DURATIONS> targetFixedFrameDurationsS;

    uint64_t fixedStartUs = 0u;
    Emergence::RunningAverage<30u> averageFixedRealDurationS;

    uint64_t normalTimeUs = 0u;
    float normalDurationS = 0.0f;

    /// Offset from engine-provided clock zero to normal time zero.
    uint64_t normalTimeOffsetUs = 0u;
    Emergence::RunningAverage<30u> averageNormalRealDurationS;

    struct Reflection final
    {
        Emergence::StandardLayout::FieldId fixedTimeUs;
        Emergence::StandardLayout::FieldId fixedDurationS;
        std::array<Emergence::StandardLayout::FieldId, MAXIMUM_TARGET_FIXED_DURATIONS> targetFixedFrameDurationsS;
        Emergence::StandardLayout::FieldId fixedStartUs;
        Emergence::StandardLayout::FieldId averageFixedRealDurationS;
        Emergence::StandardLayout::FieldId normalTimeUs;
        Emergence::StandardLayout::FieldId normalDurationS;
        Emergence::StandardLayout::FieldId normalTimeOffsetUs;
        Emergence::StandardLayout::FieldId averageNormalRealDurationS;
        Emergence::StandardLayout::Mapping mapping;
    };

    static const Reflection &Reflect () noexcept;
};

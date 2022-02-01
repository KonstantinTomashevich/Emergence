#pragma once

#include <cstdint>

#include <StandardLayout/Mapping.hpp>

#include <Utility/RunningAverage.hpp>

namespace Emergence::Celerity
{
/// \note For small periods of time floats are the most suitable format, therefore we them use for frame durations.
///       But for long periods of time (more than several hours) they may become a bit unstable, therefore we
///       use 64-bit nanosecond counters.
struct TimeSingleton final
{
    static constexpr std::size_t MAXIMUM_TARGET_FIXED_DURATIONS = 3u;

    /// \details Is not constrained to Time::NanosecondsSinceStartup.
    uint64_t normalTimeNs = 0u;

    /// \details Unaffected by ::timeSpeed mechanics, taken directly from Time::NanosecondsSinceStartup.
    ///          Used to correctly calculate deltas and advance time.
    uint64_t realNormalTimeNs = 0u;

    /// \details Is not constrained to Time::NanosecondsSinceStartup.
    uint64_t fixedTimeNs = 0u;

    float normalDurationS = 0.0f;

    /// \brief ::normalDurationS, but unaffected by ::timeSpeed.
    /// \details Useful for visuals, that should not be affected by world speed, like UI.
    float realNormalDurationS = 0.0f;

    /// Will be selected automatically from ::targetFixedFrameDurationsS.
    float fixedDurationS = 0.0f;

    Emergence::Container::InplaceVector<float, MAXIMUM_TARGET_FIXED_DURATIONS> targetFixedFrameDurationsS {
        1000.0f / 120.0f, 1000.0f / 60.0f, 1000.0f / 30.0f};

    /// \brief Allows gameplay mechanics to slowdown or speedup time.
    /// \invariant >= 0.0f
    float timeSpeed = 1.0f;

    /// \brief Average time of frames, where at least one normal and at least fixed update was executed.
    /// \details Used to adjust ::fixedDurationS in order to avoid death spiraling.
    Emergence::RunningAverage<30u> averageFullFrameRealDurationS;

    struct Reflection final
    {
        Emergence::StandardLayout::FieldId normalTimeNs;
        Emergence::StandardLayout::FieldId realNormalTimeNs;
        Emergence::StandardLayout::FieldId fixedTimeNs;

        Emergence::StandardLayout::FieldId normalDurationS;
        Emergence::StandardLayout::FieldId realNormalDurationS;
        Emergence::StandardLayout::FieldId fixedDurationS;
        std::array<Emergence::StandardLayout::FieldId, MAXIMUM_TARGET_FIXED_DURATIONS> targetFixedFrameDurationsS;

        Emergence::StandardLayout::FieldId timeSpeed;

        Emergence::StandardLayout::FieldId averageFullFrameRealDurationS;

        Emergence::StandardLayout::Mapping mapping;
    };

    static const Reflection &Reflect () noexcept;
};
} // namespace Emergence::Celerity

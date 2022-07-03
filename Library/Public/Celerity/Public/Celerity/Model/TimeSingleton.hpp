#pragma once

#include <cstdint>

#include <Math/RunningAverage.hpp>

#include <StandardLayout/Mapping.hpp>

namespace Emergence::Celerity
{
/// \brief Provides information about time to tasks.
/// \note For small periods of time floats are the most suitable format, therefore we them use for frame durations.
///       But for long periods of time (more than several hours) they may become a bit unstable, therefore we
///       use 64-bit nanosecond counters.
/// \warning This singleton is modified by World outside of pipeline execution,
///          therefore OnChange events do not work with it.
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

    /// \brief Time elapsed from previous normal update.
    float normalDurationS = 0.0f;

    /// \brief ::normalDurationS, but unaffected by ::timeSpeed.
    /// \details Useful for visuals, that should not be affected by world speed, like UI.
    float realNormalDurationS = 0.0f;

    /// \brief Will be selected automatically from ::targetFixedFrameDurationsS.
    float fixedDurationS = 0.0f;

    /// \brief Provides min-to-max sorted list of possible fixed frame rates to the framework.
    /// \details Celerity selects smallest fixed frame rate duration that does not lead to death spiralling.
    Container::InplaceVector<float, MAXIMUM_TARGET_FIXED_DURATIONS> targetFixedFrameDurationsS;

    /// \brief Allows gameplay mechanics to slowdown or speedup time.
    /// \invariant >= 0.0f
    float timeSpeed = 1.0f;

    /// \brief Average time of frames, where at least one normal and at least fixed update was executed.
    /// \details Used to adjust ::fixedDurationS in order to avoid death spiraling.
    Math::RunningAverage<30u> averageFullFrameRealDurationS;

    struct Reflection final
    {
        StandardLayout::FieldId normalTimeNs;
        StandardLayout::FieldId realNormalTimeNs;
        StandardLayout::FieldId fixedTimeNs;

        StandardLayout::FieldId normalDurationS;
        StandardLayout::FieldId realNormalDurationS;
        StandardLayout::FieldId fixedDurationS;
        StandardLayout::FieldId targetFixedFrameDurationsS;

        StandardLayout::FieldId timeSpeed;

        StandardLayout::FieldId averageFullFrameRealDurationS;

        StandardLayout::Mapping mapping;
    };

    static const Reflection &Reflect () noexcept;
};
} // namespace Emergence::Celerity

#pragma once

#include <cstdint>

namespace Emergence
{
class Time final
{
public:
    Time () = delete;

    /// \return Time elapsed since program startup in nanoseconds.
    /// \details Precise enough for ~500 years of gameplay.
    ///          I might be overly pessimistic, but I don't think anyone would leave game running for so long.
    static uint64_t NanosecondsSinceStartup () noexcept;

    /// \brief Allows to override nanoseconds counter with given fixed value. Useful for tests.
    static void Override (uint64_t _ns) noexcept;

    static void LiftOverride () noexcept;
};
} // namespace Emergence

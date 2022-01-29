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
};
} // namespace Emergence

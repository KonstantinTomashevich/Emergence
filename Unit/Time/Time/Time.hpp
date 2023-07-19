#pragma once

#include <TimeApi.hpp>

#include <cstdint>

namespace Emergence
{
class TimeApi Time final
{
public:
    Time () = delete;

    /// \return Time elapsed since program startup in nanoseconds.
    /// \details Precise enough for ~500 years of gameplay.
    ///          I might be overly pessimistic, but I don't think anyone would leave game running for so long.
    static std::uint64_t NanosecondsSinceStartup () noexcept;

    /// \brief Allows to override nanoseconds counter with given fixed value. Useful for tests.
    static void Override (std::uint64_t _ns) noexcept;

    static void LiftOverride () noexcept;
};
} // namespace Emergence

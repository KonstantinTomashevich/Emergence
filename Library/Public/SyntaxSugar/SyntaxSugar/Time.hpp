#pragma once

#include <cstdint>

namespace Emergence
{
class Time final
{
public:
    Time () = delete;

    /// \return Time elapsed since program startup in nanoseconds.
    static uint64_t NanosecondsSinceStartup () noexcept;
};
} // namespace Emergence

#include <chrono>

#include <Time/Time.hpp>

namespace Emergence
{
struct TimeData final
{
    std::chrono::high_resolution_clock::time_point startupTime = std::chrono::high_resolution_clock::now ();
    bool frozen = false;
    std::uint64_t frozenValue = 0u;
};

// We need to enclose this data inside function to make sure that it works properly during static initialization.
static TimeData &AccessTimeData ()
{
    static TimeData data;
    return data;
}

uint64_t Time::NanosecondsSinceStartup () noexcept
{
    TimeData &data = AccessTimeData ();
    if (data.frozen)
    {
        [[unlikely]] return data.frozenValue;
    }
    else
    {
        return static_cast<std::uint64_t> ((std::chrono::high_resolution_clock::now () - data.startupTime).count ());
    }
}

void Time::Override (std::uint64_t _ns) noexcept
{
    TimeData &data = AccessTimeData ();
    data.frozen = true;
    data.frozenValue = _ns;
}

void Time::LiftOverride () noexcept
{
    AccessTimeData ().frozen = false;
}

// Function-static variables initialization is done on first call,
// therefore we must guarantee that function is called at least once during startup.
[[maybe_unused]] static std::uint64_t ensureThatStartupTimeIsRecorded = Time::NanosecondsSinceStartup ();
} // namespace Emergence

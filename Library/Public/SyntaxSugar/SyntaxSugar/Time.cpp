#include <chrono>

#include <SyntaxSugar/Time.hpp>

namespace Emergence
{
uint64_t Time::NanosecondsSinceStartup () noexcept
{
    static std::chrono::high_resolution_clock::time_point startup = std::chrono::high_resolution_clock::now ();
    return static_cast<uint64_t> ((std::chrono::high_resolution_clock::now () - startup).count ());
}

// Function-static variables initialization is done on first call,
// therefore we must guarantee that function is called at least once during startup.
[[maybe_unused]] static uint64_t ensureThatStartupTimeIsRecorded = Time::NanosecondsSinceStartup ();
} // namespace Emergence

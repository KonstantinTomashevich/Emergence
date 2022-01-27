#include <thread>

#include <SyntaxSugar/AtomicFlagGuard.hpp>

namespace Emergence
{
AtomicFlagGuard::AtomicFlagGuard (std::atomic_flag &_flag) noexcept : flag (_flag)
{
    while (flag.test_and_set (std::memory_order_acquire))
    {
        std::this_thread::yield ();
    }
}

AtomicFlagGuard::~AtomicFlagGuard () noexcept
{
    flag.clear (std::memory_order::release);
}
} // namespace Emergence

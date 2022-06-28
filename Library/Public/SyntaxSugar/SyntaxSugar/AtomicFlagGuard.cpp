#include <thread>

#include <SyntaxSugar/AtomicFlagGuard.hpp>

namespace Emergence
{
AtomicFlagGuard::AtomicFlagGuard (std::atomic_flag &_flag) noexcept : flag (_flag)
{
    LockAtomicFlag (_flag);
}

AtomicFlagGuard::~AtomicFlagGuard () noexcept
{
    UnlockAtomicFlag (flag);
}

void LockAtomicFlag (std::atomic_flag &_flag) noexcept
{
    while (_flag.test_and_set (std::memory_order_acquire))
    {
        std::this_thread::yield ();
    }
}

void UnlockAtomicFlag (std::atomic_flag &_flag) noexcept
{
    _flag.clear (std::memory_order::release);
}
} // namespace Emergence

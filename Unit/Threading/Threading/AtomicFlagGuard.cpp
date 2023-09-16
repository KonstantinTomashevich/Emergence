#include <thread>

#include <CPU/Profiler.hpp>

#include <Threading/AtomicFlagGuard.hpp>

namespace Emergence
{
AtomicFlagGuard::AtomicFlagGuard (std::atomic_flag &_flag) noexcept
    : flag (_flag)
{
    LockAtomicFlag (_flag);
}

AtomicFlagGuard::~AtomicFlagGuard () noexcept
{
    UnlockAtomicFlag (flag);
}

static const char *const LOCK_SECTION_NAME = "AtomicFlagLock";

void LockAtomicFlag (std::atomic_flag &_flag) noexcept
{
    static CPU::Profiler::SectionDefinition lockSection {LOCK_SECTION_NAME, 0xFF990000u};
    CPU::Profiler::SectionInstance section {lockSection};

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

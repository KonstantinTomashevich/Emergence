#pragma once

#include <atomic>

#include <Threading/AtomicFlagGuard.hpp>

namespace Emergence::Memory::Profiler::Original
{
/// \brief Currently memory profiling logic operates under one spinlock.
class ProfilingLock final
{
private:
    static std::atomic_flag &GetSharedLock () noexcept;

    AtomicFlagGuard guard {GetSharedLock ()};
};
} // namespace Emergence::Memory::Profiler::Original

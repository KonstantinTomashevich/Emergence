#include <Memory/Profiler/Original/ProfilingLock.hpp>

namespace Emergence::Memory::Profiler::Original
{
std::atomic_flag &ProfilingLock::GetSharedLock () noexcept
{
    static std::atomic_flag lock;
    return lock;
}
} // namespace Emergence::Memory::Profiler::Original

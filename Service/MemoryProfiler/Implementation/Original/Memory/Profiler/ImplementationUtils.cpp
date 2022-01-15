#include <Memory/Profiler/ImplementationUtils.hpp>

namespace Emergence::Memory::Profiler
{
Profiler::AllocationGroup ImplementationUtils::ToServiceFormat (Original::AllocationGroup *_group) noexcept
{
    return Profiler::AllocationGroup {_group};
}
} // namespace Emergence::Memory::Profiler

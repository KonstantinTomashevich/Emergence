#include <Container/String.hpp>

namespace Emergence::Memory
{
Profiler::AllocationGroup DefaultAllocationGroup<char>::Get () noexcept
{
    static Profiler::AllocationGroup group {Profiler::AllocationGroup::Root (), UniqueString {"UntrackedString"}};
    return group;
}
} // namespace Emergence::Memory

#include <Task/Collection.hpp>

namespace Emergence::Task
{
using namespace Memory::Literals;

Memory::Profiler::AllocationGroup Collection::GetDefaultAllocationGroup () noexcept
{
    static Memory::Profiler::AllocationGroup group {Memory::Profiler::AllocationGroup::Root (), "TaskCollection"_us};
    return group;
}
} // namespace Emergence::Task

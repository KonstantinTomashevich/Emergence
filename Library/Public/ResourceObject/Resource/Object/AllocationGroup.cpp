#include <Resource/Object/AllocationGroup.hpp>

namespace Emergence::Resource::Object
{
using namespace Memory::Literals;

Memory::Profiler::AllocationGroup GetRootAllocationGroup () noexcept
{
    static Memory::Profiler::AllocationGroup group {
        Memory::Profiler::AllocationGroup {Memory::Profiler::AllocationGroup::Root (), "Asset"_us}, "Object"_us};
    return group;
}
} // namespace Emergence::Resource::Object

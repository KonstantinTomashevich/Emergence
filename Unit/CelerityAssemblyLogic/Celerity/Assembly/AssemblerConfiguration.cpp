#include <Celerity/Assembly/AssemblerConfiguration.hpp>

namespace Emergence::Celerity
{
Memory::Profiler::AllocationGroup GetAssemblerConfigurationAllocationGroup () noexcept
{
    static Memory::Profiler::AllocationGroup group {Memory::UniqueString {"AssemblerConfiguration"}};
    return group;
}
} // namespace Emergence::Celerity

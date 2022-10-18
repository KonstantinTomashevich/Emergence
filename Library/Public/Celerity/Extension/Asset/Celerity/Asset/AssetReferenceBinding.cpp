#include <Celerity/Asset/AssetReferenceBinding.hpp>

namespace Emergence::Celerity
{
Memory::Profiler::AllocationGroup AssetBindingAllocationGroup () noexcept
{
    static Memory::Profiler::AllocationGroup group {Memory::Profiler::AllocationGroup::Root (),
                                                    Memory::UniqueString {"CelerityAssetBinding"}};
    return group;
}
} // namespace Emergence::Celerity

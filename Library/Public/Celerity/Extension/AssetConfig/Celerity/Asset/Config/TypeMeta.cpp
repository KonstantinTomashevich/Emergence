#include <Celerity/Asset/Config/TypeMeta.hpp>

namespace Emergence::Memory
{
Profiler::AllocationGroup DefaultAllocationGroup<Celerity::AssetConfigTypeMeta>::Get () noexcept
{
    return Profiler::AllocationGroup {UniqueString {"CelerityAssetConfigTypeMeta"}};
}
} // namespace Emergence::Memory

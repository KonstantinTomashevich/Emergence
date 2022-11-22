#include <Celerity/Resource/Config/TypeMeta.hpp>

namespace Emergence::Memory
{
Profiler::AllocationGroup DefaultAllocationGroup<Celerity::ResourceConfigTypeMeta>::Get () noexcept
{
    return Profiler::AllocationGroup {UniqueString {"CelerityResourceConfigTypeMeta"}};
}
} // namespace Emergence::Memory

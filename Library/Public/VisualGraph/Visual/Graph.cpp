#include <Visual/Graph.hpp>

namespace Emergence
{
namespace VisualGraph
{
Memory::Profiler::AllocationGroup GetDefaultAllocationGroup () noexcept
{
    static const Memory::Profiler::AllocationGroup ALLOCATION_GROUP (Memory::Profiler::AllocationGroup::Root (),
                                                                     Memory::UniqueString {"VisualGraph"});
    return ALLOCATION_GROUP;
}
} // namespace VisualGraph

namespace Memory
{
Profiler::AllocationGroup DefaultAllocationGroup<VisualGraph::Node>::Get () noexcept
{
    return VisualGraph::GetDefaultAllocationGroup ();
}

Profiler::AllocationGroup DefaultAllocationGroup<VisualGraph::Edge>::Get () noexcept
{
    return VisualGraph::GetDefaultAllocationGroup ();
}

Profiler::AllocationGroup DefaultAllocationGroup<VisualGraph::Graph>::Get () noexcept
{
    return VisualGraph::GetDefaultAllocationGroup ();
}
} // namespace Memory
} // namespace Emergence

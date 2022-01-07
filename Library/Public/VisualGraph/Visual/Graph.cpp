#include <Visual/Graph.hpp>

namespace Emergence
{
namespace VisualGraph
{
static const Memory::Profiler::AllocationGroup ALLOCATION_GROUP (Memory::Profiler::AllocationGroup::Root (),
                                                                 Memory::UniqueString {"VisualGraph"});

Memory::Profiler::AllocationGroup GetDefaultAllocationGroup () noexcept
{
    return ALLOCATION_GROUP;
}
} // namespace VisualGraph

namespace Memory
{
Profiler::AllocationGroup DefaultAllocationGroup<VisualGraph::Node>::Get () noexcept
{
    return VisualGraph::ALLOCATION_GROUP;
}

Profiler::AllocationGroup DefaultAllocationGroup<VisualGraph::Edge>::Get () noexcept
{
    return VisualGraph::ALLOCATION_GROUP;
}

Profiler::AllocationGroup DefaultAllocationGroup<VisualGraph::Graph>::Get () noexcept
{
    return VisualGraph::ALLOCATION_GROUP;
}
} // namespace Memory
} // namespace Emergence

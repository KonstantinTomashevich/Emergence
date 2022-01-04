#include <Visual/Graph.hpp>

namespace Emergence::Memory
{
using namespace Literals;

static const Profiler::AllocationGroup ALLOCATION_GROUP (Profiler::AllocationGroup::Root (), "VisualGraph"_us);

Profiler::AllocationGroup DefaultAllocationGroup<VisualGraph::Node>::Get () noexcept
{
    return ALLOCATION_GROUP;
}

Profiler::AllocationGroup DefaultAllocationGroup<VisualGraph::Edge>::Get () noexcept
{
    return ALLOCATION_GROUP;
}

Profiler::AllocationGroup DefaultAllocationGroup<VisualGraph::Graph>::Get () noexcept
{
    return ALLOCATION_GROUP;
}
} // namespace Emergence::Memory

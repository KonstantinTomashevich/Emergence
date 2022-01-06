#include <Celerity/Pipeline.hpp>
#include <Celerity/World.hpp>

#include <Memory/Profiler/AllocationGroup.hpp>

namespace Emergence::Celerity
{
static Warehouse::Registry ConstructInsideGroup (Memory::UniqueString _worldName)
{
    auto placeholder = Memory::Profiler::AllocationGroup {_worldName}.PlaceOnTop ();
    return Warehouse::Registry {_worldName};
}

World::World (Memory::UniqueString _name) noexcept
    : registry (ConstructInsideGroup (_name)),
      pipelineHeap (Memory::Profiler::AllocationGroup {Memory::Profiler::AllocationGroup {_name},
                                                       Memory::UniqueString {"Pipelines"}}),
      pipelines (pipelineHeap)
{
}

World::~World ()
{
    for (Pipeline *pipeline : pipelines)
    {
        pipeline->~Pipeline ();
        pipelineHeap.Release (pipeline, sizeof (Pipeline));
    }
}

std::uintptr_t World::GetNextObjectId () noexcept
{
    return objectIdCounter++;
}

Warehouse::FetchSingletonQuery World::FetchSingletonExternally (const StandardLayout::Mapping &_mapping) noexcept
{
    return registry.FetchSingleton (_mapping);
}

Warehouse::ModifySingletonQuery World::ModifySingletonExternally (const StandardLayout::Mapping &_mapping) noexcept
{
    return registry.ModifySingleton (_mapping);
}

Pipeline *World::AddPipeline (const Task::Collection &_collection, std::size_t _maximumChildThreads)
{
    return pipelines.emplace_back (new (pipelineHeap.Acquire (sizeof (Pipeline)))
                                       Pipeline {_collection, _maximumChildThreads});
}
} // namespace Emergence::Celerity

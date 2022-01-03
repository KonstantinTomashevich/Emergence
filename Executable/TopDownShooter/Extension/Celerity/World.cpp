#include <Celerity/Pipeline.hpp>
#include <Celerity/World.hpp>

#include <Memory/Profiler/AllocationGroup.hpp>

namespace Emergence::Celerity
{
namespace MP = Memory::Profiler;

static const Memory::UniqueString PIPELINES_VECTOR {"PipelineVector"};

World::World (Memory::UniqueString _name) noexcept
    : registry (_name),
      pipelines (MP::ConstructWithinGroup<decltype (pipelines)> (_name, PIPELINES_VECTOR))
{
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
} // namespace Emergence::Celerity

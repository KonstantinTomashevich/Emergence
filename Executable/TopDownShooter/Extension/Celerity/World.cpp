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
      pipelines (Memory::Profiler::AllocationGroup {_name})
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

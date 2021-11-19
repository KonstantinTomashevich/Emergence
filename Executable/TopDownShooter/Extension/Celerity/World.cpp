#include <Celerity/Pipeline.hpp>
#include <Celerity/World.hpp>

namespace Emergence::Celerity
{
World::World (std::string _name) noexcept : registry (std::move (_name))
{
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

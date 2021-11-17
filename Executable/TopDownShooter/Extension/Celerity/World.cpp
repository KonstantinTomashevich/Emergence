#include <Celerity/Pipeline.hpp>
#include <Celerity/World.hpp>

namespace Emergence::Celerity
{
World::World (std::string _name) noexcept : registry (std::move (_name))
{
}

Warehouse::FetchSingletonQuery World::FetchSingletonForExternalUse (const StandardLayout::Mapping &_mapping) noexcept
{
    return registry.FetchSingleton (_mapping);
}
} // namespace Emergence::Celerity

#include <Assert/Assert.hpp>

#include <Container/MappingRegistry.hpp>

namespace Emergence::Container
{
void MappingRegistry::Register (StandardLayout::Mapping _mapping) noexcept
{
    EMERGENCE_ASSERT (!registry.contains (_mapping.GetName ()));
    registry[_mapping.GetName ()] = std::move (_mapping);
}

StandardLayout::Mapping MappingRegistry::Get (Memory::UniqueString _name) const noexcept
{
    if (auto iterator = registry.find (_name); iterator != registry.end ())
    {
        return iterator->second;
    }

    return {};
}

const HashMap<Memory::UniqueString, StandardLayout::Mapping> &MappingRegistry::GetRegistry () const noexcept
{
    return registry;
}
} // namespace Emergence::Container

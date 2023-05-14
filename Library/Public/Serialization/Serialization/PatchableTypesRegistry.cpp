#include <Assert/Assert.hpp>

#include <Serialization/PatchableTypesRegistry.hpp>

namespace Emergence::Serialization
{
void PatchableTypesRegistry::Register (StandardLayout::Mapping _mapping) noexcept
{
    EMERGENCE_ASSERT (!registry.contains (_mapping.GetName ()));
    registry[_mapping.GetName ()] = std::move (_mapping);
}

StandardLayout::Mapping PatchableTypesRegistry::Get (Memory::UniqueString _name) const noexcept
{
    if (auto iterator = registry.find (_name); iterator != registry.end ())
    {
        return iterator->second;
    }

    return {};
}
} // namespace Emergence::Serialization

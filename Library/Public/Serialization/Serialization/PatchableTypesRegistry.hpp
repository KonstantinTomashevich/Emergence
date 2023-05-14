#pragma once

#include <Container/HashMap.hpp>
#include <Container/Optional.hpp>

#include <Memory/UniqueString.hpp>

#include <StandardLayout/Mapping.hpp>

namespace Emergence::Serialization
{
/// \brief Lists all types that can be used to create StandardLayout::Patch's during deserialization.
class PatchableTypesRegistry final
{
public:
    /// \brief Registers given type as suitable for creating patches.
    void Register (StandardLayout::Mapping _mapping) noexcept;

    /// \return Mapping with given name or invalid Mapping if type is not supported from creating patches.
    [[nodiscard]] StandardLayout::Mapping Get (Memory::UniqueString _name) const noexcept;

private:
    Container::HashMap<Memory::UniqueString, StandardLayout::Mapping> registry {
        Memory::Profiler::AllocationGroup {Memory::UniqueString {"PatchableTypesRegistry"}}};
};
} // namespace Emergence::Serialization

#pragma once

#include <ContainerApi.hpp>

#include <Container/HashMap.hpp>
#include <Container/Optional.hpp>

#include <Memory/UniqueString.hpp>

#include <StandardLayout/Mapping.hpp>

namespace Emergence::Container
{
/// \brief Simple utility class for storing map of named mappings.
class ContainerApi MappingRegistry final
{
public:
    /// \brief Registers given type.
    void Register (StandardLayout::Mapping _mapping) noexcept;

    /// \return Mapping with given name or invalid Mapping if type is not registered.
    [[nodiscard]] StandardLayout::Mapping Get (Memory::UniqueString _name) const noexcept;

    /// \return Whole registry map.
    [[nodiscard]] const Container::HashMap<Memory::UniqueString, StandardLayout::Mapping> &GetRegistry ()
        const noexcept;

private:
    Container::HashMap<Memory::UniqueString, StandardLayout::Mapping> registry {
        Memory::Profiler::AllocationGroup {Memory::UniqueString {"MappingRegistry"}}};
};
} // namespace Emergence::Container

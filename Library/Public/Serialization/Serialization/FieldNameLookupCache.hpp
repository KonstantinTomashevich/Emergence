#pragma once

#include <Container/HashMap.hpp>

#include <Memory/UniqueString.hpp>

#include <StandardLayout/Mapping.hpp>

namespace Emergence::Serialization
{
/// \brief Provides field name to field info query caching for
///        faster execution of subsequent deserialization operations.
class FieldNameLookupCache final
{
public:
    /// \brief Constructs cache for given type.
    explicit FieldNameLookupCache (StandardLayout::Mapping _mapping) noexcept;

    /// \return Handle to field that has given name. Invalid handle if field with this name was not found.
    StandardLayout::Field Lookup (Memory::UniqueString _name) noexcept;

    /// \return Mapping of type for which this cache is constructed.
    [[nodiscard]] const StandardLayout::Mapping &GetTypeMapping () const noexcept;

private:
    StandardLayout::Mapping mapping;
    Container::HashMap<Memory::UniqueString, StandardLayout::Field> cache {
        Memory::Profiler::AllocationGroup {Memory::UniqueString {"FieldNameLookupCache"}}};
};
} // namespace Emergence::Serialization

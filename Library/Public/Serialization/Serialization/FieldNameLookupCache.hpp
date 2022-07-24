#pragma once

#include <Container/HashMap.hpp>

#include <Memory/UniqueString.hpp>

#include <StandardLayout/Mapping.hpp>

namespace Emergence::Serialization
{
class FieldNameLookupCache final
{
public:
    explicit FieldNameLookupCache (const StandardLayout::Mapping &_mapping) noexcept;

    StandardLayout::Field Lookup (Memory::UniqueString _name) noexcept;

    [[nodiscard]] const StandardLayout::Mapping &GetTypeMapping () const noexcept;

private:
    StandardLayout::Mapping mapping;
    Container::HashMap<Memory::UniqueString, StandardLayout::Field> cache;
};
} // namespace Emergence::Serialization

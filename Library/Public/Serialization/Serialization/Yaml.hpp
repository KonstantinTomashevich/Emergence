#pragma once

#include <istream>
#include <ostream>

#include <Serialization/FieldNameLookupCache.hpp>

#include <StandardLayout/Mapping.hpp>

namespace Emergence::Serialization::Yaml
{
void SerializeObject (std::ostream &_output, const void *_object, const StandardLayout::Mapping &_mapping) noexcept;

bool DeserializeObject (std::istream &_input, void *_object, const StandardLayout::Mapping &_mapping) noexcept;

bool DeserializeObject (std::istream &_input, void *_object, const StandardLayout::Mapping &_mapping, FieldNameLookupCache &_cache) noexcept;
} // namespace Emergence::Serialization::Yaml

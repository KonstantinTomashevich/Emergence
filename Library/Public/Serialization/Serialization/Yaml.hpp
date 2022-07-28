#pragma once

#include <istream>
#include <ostream>

#include <Serialization/FieldNameLookupCache.hpp>

#include <StandardLayout/Mapping.hpp>
#include <StandardLayout/Patch.hpp>
#include <StandardLayout/PatchBuilder.hpp>

namespace Emergence::Serialization::Yaml
{
void SerializeObject (std::ostream &_output, const void *_object, const StandardLayout::Mapping &_mapping) noexcept;

bool DeserializeObject (std::istream &_input, void *_object, FieldNameLookupCache &_cache) noexcept;

void SerializePatch (std::ostream &_output, const StandardLayout::Patch &_patch) noexcept;

bool DeserializePatch (std::istream &_input,
                       StandardLayout::PatchBuilder &_builder,
                       FieldNameLookupCache &_cache) noexcept;
} // namespace Emergence::Serialization::Yaml

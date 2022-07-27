#pragma once

#include <istream>
#include <ostream>

#include <Serialization/FieldNameLookupCache.hpp>

#include <StandardLayout/Mapping.hpp>
#include <StandardLayout/Patch.hpp>
#include <StandardLayout/PatchBuilder.hpp>

namespace Emergence::Serialization::Binary
{
void SerializeObject (std::ostream &_output, const void *_object, const StandardLayout::Mapping &_mapping) noexcept;

bool DeserializeObject (std::istream &_input, void *_object, const StandardLayout::Mapping &_mapping) noexcept;

void SerializePatch (std::ostream &_output, const StandardLayout::Patch &_patch) noexcept;

bool DeserializePatch (std::istream &_input,
                       StandardLayout::PatchBuilder &_builder,
                       const StandardLayout::Mapping &_mapping) noexcept;

void SerializeFastPortablePatch (std::ostream &_output, const StandardLayout::Patch &_patch) noexcept;

bool DeserializeFastPortablePatch (std::istream &_input,
                                   StandardLayout::PatchBuilder &_builder,
                                   const StandardLayout::Mapping &_mapping) noexcept;

bool DeserializeFastPortablePatch (std::istream &_input,
                                   StandardLayout::PatchBuilder &_builder,
                                   const StandardLayout::Mapping &_mapping,
                                   FieldNameLookupCache &_cache) noexcept;

// TODO: Space efficient portable patches: use enter-exit commands instead of duplicating prefixes like
//       firstLevelInnerObject.secondLevelInnerObject.firstLeafField
//       firstLevelInnerObject.secondLevelInnerObject.secondLeafField
} // namespace Emergence::Serialization::Binary

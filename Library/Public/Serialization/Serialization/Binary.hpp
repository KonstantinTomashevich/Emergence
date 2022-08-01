#pragma once

#include <istream>
#include <ostream>

#include <Serialization/FieldNameLookupCache.hpp>

#include <StandardLayout/Mapping.hpp>
#include <StandardLayout/Patch.hpp>
#include <StandardLayout/PatchBuilder.hpp>

namespace Emergence::Serialization::Binary
{
/// \brief Serializes given object of given type into given binary stream.
/// \warning Serialization is mapping-dependant: deserialization must be
///          made using exactly same mapping, otherwise data will be broken.
void SerializeObject (std::ostream &_output, const void *_object, const StandardLayout::Mapping &_mapping) noexcept;

/// \brief Deserializes data from given binary stream into given address using given mapping.
/// \invariant Serialization must be done through ::SerializeObject.
bool DeserializeObject (std::istream &_input, void *_object, const StandardLayout::Mapping &_mapping) noexcept;

/// \brief Serializes given patch into given binary stream.
/// \warning Serialization is patch-mapping-dependant: deserialization must be
///          made using exactly same mapping, otherwise data will be broken.
void SerializePatch (std::ostream &_output, const StandardLayout::Patch &_patch) noexcept;

/// \brief Builds new patch from given binary stream.
/// \invariant Serialization must be done through ::SerializePatch.
bool DeserializePatch (std::istream &_input,
                       StandardLayout::PatchBuilder &_builder,
                       const StandardLayout::Mapping &_mapping) noexcept;

/// \brief Modification of ::SerializePatch format that can be used with partially changed mappings.
/// \details Addition and removal of mapping fields between serialization and deserialization is supported.
///          It's called fast, because it is easy to serialize/deserialize patches in that format,
///          but binary size is relatively big.
/// \invariant Changing field archetypes and sizes without renaming these fields is not supported!
void SerializeFastPortablePatch (std::ostream &_output, const StandardLayout::Patch &_patch) noexcept;

/// \brief Deserializer for ::SerializeFastPortablePatch format.
/// \details Prefer using shared `_cache` to speed up deserialization of multiple patches of the same type.
bool DeserializeFastPortablePatch (std::istream &_input,
                                   StandardLayout::PatchBuilder &_builder,
                                   FieldNameLookupCache &_cache) noexcept;
} // namespace Emergence::Serialization::Binary

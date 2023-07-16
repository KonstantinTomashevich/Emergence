#pragma once

#include <istream>
#include <ostream>

#include <Container/MappingRegistry.hpp>

#include <StandardLayout/Mapping.hpp>

namespace Emergence::Serialization::Binary
{
/// \brief Adds type name info to given output stream.
void SerializeTypeName (std::ostream &_output, Memory::UniqueString _typeName) noexcept;

/// \brief Attempts to read type name from given output stream. Returns empty name on error.
Memory::UniqueString DeserializeTypeName (std::istream &_input) noexcept;

/// \brief Serializes given object of given type into given binary stream.
/// \warning Serialization is mapping-dependant: deserialization must be
///          made using exactly same mapping, otherwise data will be broken.
void SerializeObject (std::ostream &_output, const void *_object, const StandardLayout::Mapping &_mapping) noexcept;

/// \brief Deserializes data from given binary stream into given address using given mapping.
/// \invariant Serialization must be done through ::SerializeObject.
bool DeserializeObject (std::istream &_input,
                        void *_object,
                        const StandardLayout::Mapping &_mapping,
                        const Container::MappingRegistry &_patchableTypesRegistry) noexcept;
} // namespace Emergence::Serialization::Binary

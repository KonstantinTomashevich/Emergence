#pragma once

#include <SerializationApi.hpp>

#include <istream>
#include <ostream>

#include <Container/MappingRegistry.hpp>

#include <StandardLayout/Mapping.hpp>

namespace Emergence::Serialization::Yaml
{
/// \brief Adds type name info to given output stream.
SerializationApi void SerializeTypeName (std::ostream &_output, Memory::UniqueString _typeName) noexcept;

/// \brief Attempts to read type name from given output stream. Returns empty name on error.
SerializationApi Memory::UniqueString DeserializeTypeName (std::istream &_input) noexcept;

/// \brief Serializes given object of given type into given text stream using Yaml format.
SerializationApi void SerializeObject (std::ostream &_output,
                                       const void *_object,
                                       const StandardLayout::Mapping &_mapping) noexcept;

/// \brief Deserializes data from given text stream with Yaml data into given address using given mapping.
bool SerializationApi DeserializeObject (std::istream &_input,
                                         void *_object,
                                         const StandardLayout::Mapping &_mapping,
                                         const Container::MappingRegistry &_patchableTypesRegistry) noexcept;
} // namespace Emergence::Serialization::Yaml

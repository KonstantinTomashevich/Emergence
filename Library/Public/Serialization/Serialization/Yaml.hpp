#pragma once

#include <istream>
#include <ostream>

#include <Container/HashMap.hpp>
#include <Container/Optional.hpp>
#include <Container/Vector.hpp>

#include <Serialization/PatchableTypesRegistry.hpp>

#include <StandardLayout/Mapping.hpp>
#include <StandardLayout/Patch.hpp>
#include <StandardLayout/PatchBuilder.hpp>

namespace Emergence::Serialization::Yaml
{
/// \brief Serializes given object of given type into given text stream using Yaml format.
void SerializeObject (std::ostream &_output, const void *_object, const StandardLayout::Mapping &_mapping) noexcept;

/// \brief Deserializes data from given text stream with Yaml data into given address using given mapping.
bool DeserializeObject (std::istream &_input,
                        void *_object,
                        const StandardLayout::Mapping &_mapping,
                        const PatchableTypesRegistry &_patchableTypesRegistry) noexcept;
} // namespace Emergence::Serialization::Yaml

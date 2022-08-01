#pragma once

#include <istream>
#include <ostream>

#include <Container/HashMap.hpp>
#include <Container/Vector.hpp>

#include <Serialization/FieldNameLookupCache.hpp>

#include <StandardLayout/Mapping.hpp>
#include <StandardLayout/Patch.hpp>
#include <StandardLayout/PatchBuilder.hpp>

namespace Emergence::Serialization::Yaml
{
/// \brief Serializes given object of given type into given text stream using Yaml format.
void SerializeObject (std::ostream &_output, const void *_object, const StandardLayout::Mapping &_mapping) noexcept;

/// \brief Deserializes data from given text stream with Yaml data into given address using given mapping.
bool DeserializeObject (std::istream &_input, void *_object, FieldNameLookupCache &_cache) noexcept;

/// \brief Serializes given patch into given text stream using Yaml format.
void SerializePatch (std::ostream &_output, const StandardLayout::Patch &_patch) noexcept;

/// \brief Builds new patch from given text stream with Yaml data.
bool DeserializePatch (std::istream &_input,
                       StandardLayout::PatchBuilder &_builder,
                       FieldNameLookupCache &_cache) noexcept;

/// \brief Deserializes a bundle of patches from given text stream with Yaml data.
/// \details Patch bundle format allows to store several patches with different types in one file like that:
///          ```yaml
///          - type: Patch0Type
///            content:
///              # Patch content in ::SerializePatch format
///          # ...
///          - type: PatchNType
///            content:
///              # Patch content in ::SerializePatch format
///          ```
///          This is useful for storing logically connected patches of different types,
///          for example CelerityAssembly AssemblyDescriptor data.
///
///          Prefer using shared `_context` to speed up deserialization of multiple bundles.
bool DeserializePatchBundle (std::istream &_input,
                             Container::Vector<StandardLayout::Patch> &_output,
                             class BundleDeserializationContext &_context) noexcept;

/// \brief Contains caching context for ::DeserializePatchBundle.
/// \details Provides type list and allows to speed up deserialization of multiple patch bundles.
class BundleDeserializationContext final
{
public:
    /// \brief Registers given type as possible type for patches inside bundles.
    void RegisterType (const StandardLayout::Mapping &_mapping) noexcept;

    /// \return Cache for given type if it was registered.
    FieldNameLookupCache *RequestCache (Memory::UniqueString _typeName) noexcept;

private:
    Container::HashMap<Memory::UniqueString, FieldNameLookupCache> cachesByTypeName {
        Memory::Profiler::AllocationGroup {Memory::UniqueString {"BundleDeserializationContext"}}};
};
} // namespace Emergence::Serialization::Yaml

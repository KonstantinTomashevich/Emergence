#pragma once

#include <istream>
#include <ostream>

#include <Container/HashMap.hpp>
#include <Container/Optional.hpp>
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

// TODO: Refactor to more SAX-like parsing later?

/// \brief Internal type, used to hide YAML library details.
using YamlRootPlaceholder = std::array<uint8_t, sizeof (uintptr_t) * 8u>;

/// \brief Internal type, used to hide YAML library details.
using YamlIteratorPlaceholder = std::array<uint8_t, sizeof (uintptr_t) * 6u>;

/// \brief Serializes multiple objects of the same type into YAML sequence.
class ObjectBundleSerializer final
{
public:
    ObjectBundleSerializer (StandardLayout::Mapping _mapping) noexcept;

    ObjectBundleSerializer (const ObjectBundleSerializer &_other) = delete;

    ObjectBundleSerializer (ObjectBundleSerializer &&_other) = delete;

    ~ObjectBundleSerializer () noexcept;

    /// \brief Begin serialization session.
    void Begin () noexcept;

    /// \brief Append new object to the current sequence.
    void Next (const void *_object) noexcept;

    /// \brief End serialization session and write result to the output.
    void End (std::ostream &_output) noexcept;

    ObjectBundleSerializer &operator= (const ObjectBundleSerializer &_other) = delete;

    ObjectBundleSerializer &operator= (ObjectBundleSerializer &&_other) = delete;

private:
    YamlRootPlaceholder yamlRootPlaceholder;
    StandardLayout::Mapping mapping;
};

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
class PatchBundleDeserializer final
{
public:
    PatchBundleDeserializer () noexcept;

    PatchBundleDeserializer (const PatchBundleDeserializer &_other) = delete;

    PatchBundleDeserializer (PatchBundleDeserializer &&_other) = delete;

    ~PatchBundleDeserializer () noexcept;

    /// \brief Registers given type as possible type for patches inside bundles.
    void RegisterType (const StandardLayout::Mapping &_mapping) noexcept;

    /// \brief Starts new bundle deserialization session.
    /// \return Whether session was started successfully.
    bool Begin (std::istream &_input) noexcept;

    /// \return Whether bundle has more unread patches.
    [[nodiscard]] bool HasNext () const noexcept;

    /// \brief Extracts next patch from bundle if possible.
    /// \return Next patch or `nullopt` if error happened.
    Container::Optional<StandardLayout::Patch> Next () noexcept;

    /// \brief Ends current bundle deserialization session.
    /// \details Safely processes case when there is no active deserialization session.
    void End () noexcept;

    PatchBundleDeserializer &operator= (const PatchBundleDeserializer &_other) = delete;

    PatchBundleDeserializer &operator= (PatchBundleDeserializer &&_other) = delete;

private:
    FieldNameLookupCache *RequestCache (Memory::UniqueString _typeName) noexcept;

    YamlRootPlaceholder yamlRootPlaceholder;
    YamlIteratorPlaceholder yamlIteratorPlaceholder;

    Container::HashMap<Memory::UniqueString, FieldNameLookupCache> cachesByTypeName {
        Memory::Profiler::AllocationGroup {Memory::UniqueString {"PatchBundleDeserializer"}}};
};
} // namespace Emergence::Serialization::Yaml

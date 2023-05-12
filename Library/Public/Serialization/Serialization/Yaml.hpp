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

/// \brief Base class for serializers that store multiple items, for example multiple objects or patches.
class BundleSerializerBase
{
public:
    BundleSerializerBase () noexcept;

    BundleSerializerBase (const BundleSerializerBase &_other) = delete;

    BundleSerializerBase (BundleSerializerBase &&_other) = delete;

    ~BundleSerializerBase () noexcept;

    /// \brief Begin serialization session.
    void Begin () noexcept;

    /// \brief End serialization session and write result to the output.
    void End (std::ostream &_output) noexcept;

    BundleSerializerBase &operator= (const BundleSerializerBase &_other) = delete;

    BundleSerializerBase &operator= (BundleSerializerBase &&_other) = delete;

protected:
    YamlRootPlaceholder yamlRootPlaceholder;
};

/// \brief Base class for deserializers that extract multiple items, for example multiple objects or patches.
class BundleDeserializerBase
{
public:
    BundleDeserializerBase () noexcept;

    BundleDeserializerBase (const BundleDeserializerBase &_other) = delete;

    BundleDeserializerBase (BundleDeserializerBase &&_other) = delete;

    ~BundleDeserializerBase () noexcept;

    /// \brief Starts new bundle deserialization session.
    /// \return Whether session was started successfully.
    bool Begin (std::istream &_input) noexcept;

    /// \return Whether bundle has more unread items.
    [[nodiscard]] bool HasNext () const noexcept;

    /// \brief Ends current bundle deserialization session.
    /// \details Safely processes case when there is no active deserialization session.
    void End () noexcept;

    BundleDeserializerBase &operator= (const BundleDeserializerBase &_other) = delete;

    BundleDeserializerBase &operator= (BundleDeserializerBase &&_other) = delete;

protected:
    YamlRootPlaceholder yamlRootPlaceholder;
    YamlIteratorPlaceholder yamlIteratorPlaceholder;
};

/// \brief Serializes multiple objects of the same type into YAML sequence.
class ObjectBundleSerializer final : public BundleSerializerBase
{
public:
    ObjectBundleSerializer (StandardLayout::Mapping _mapping) noexcept;

    /// \brief Append new object to the current sequence.
    void Next (const void *_object) noexcept;

private:
    StandardLayout::Mapping mapping;
};

/// \brief Deserializes multiple objects from YAML sequence.
class ObjectBundleDeserializer final : public BundleDeserializerBase
{
public:
    ObjectBundleDeserializer (StandardLayout::Mapping _mapping) noexcept;

    /// \brief Extracts next object from bundle if possible and writes its data to given address.
    /// \return Whether extraction has been completed successfully.
    bool Next (void *_object) noexcept;

private:
    FieldNameLookupCache fieldNameLookupCache;
};

/// \brief Serializes a bundle of patches to given text stream as YAML sequence.
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
class PatchBundleSerializer final : public BundleSerializerBase
{
public:
    /// \brief Append new patch to the current sequence.
    void Next (const StandardLayout::Patch &_patch) noexcept;

private:
    StandardLayout::Mapping mapping;
};

/// \brief Deserializes a bundle of patches from given text stream with Yaml data.
/// \details See PatchBundleSerializer for details about format.
class PatchBundleDeserializer final : public BundleDeserializerBase
{
public:
    /// \brief Registers given type as possible type for patches inside bundles.
    void RegisterType (const StandardLayout::Mapping &_mapping) noexcept;

    /// \brief Extracts next patch from bundle if possible.
    /// \return Next patch or `nullopt` if error happened.
    Container::Optional<StandardLayout::Patch> Next () noexcept;

private:
    FieldNameLookupCache *RequestCache (Memory::UniqueString _typeName) noexcept;

    Container::HashMap<Memory::UniqueString, FieldNameLookupCache> cachesByTypeName {
        Memory::Profiler::AllocationGroup {Memory::UniqueString {"PatchBundleDeserializer"}}};
};

/// \brief Serializer for string mappings: pairs of key string and value string.
class StringMappingSerializer final : public BundleSerializerBase
{
public:
    /// \brief Append new mapping to the current sequence.
    void Next (Memory::UniqueString _key, const Container::Utf8String &_value) noexcept;
};

/// \brief Deserializer for strings mappings. See StringMappingSerializer.
class StringMappingDeserializer : public BundleDeserializerBase
{
public:
    /// \brief Attempts to extract next string mapping from bundle if possible.
    bool Next (Memory::UniqueString &_keyOutput, Container::Utf8String &_valueOutput) noexcept;
};
} // namespace Emergence::Serialization::Yaml

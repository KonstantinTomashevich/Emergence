#pragma once

#include <istream>
#include <ostream>

#include <Container/Optional.hpp>
#include <Container/String.hpp>

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

/// \brief Serializes given string into given output stream.
void SerializeString (std::ostream &_output, const char *_string) noexcept;

/// \brief Attempts to deserialize string from given input into given output instance.
/// \invariant Serialization must be done through ::SerializeString.
bool DeserializeString (std::istream &_input, Container::String &_stringOutput) noexcept;

/// \brief Serializes given patch into given binary stream.
/// \warning Serialization is patch-mapping-dependant: deserialization must be
///          made using exactly same mapping, otherwise data will be broken.
///          Therefore it's advised to only use this type of serialization for
///          shipping final assets and never for storing development assets.
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

/// \brief Serializes a bundle of patches to given binary stream in ::SerializePatch format.
class PatchBundleSerializer final
{
public:
    /// \brief Starts new bundle serialization session.
    /// \invariant Output stream existence must be guaranteed by user until session is finished.
    void Begin (std::ostream &_output) noexcept;

    /// \brief Serializes next patch into bundle output stream.
    void Next (const StandardLayout::Patch &_patch) noexcept;

    /// \brief Ends current bundle serialization session.
    void End () noexcept;

private:
    std::ostream *output = nullptr;
};

/// \brief Deserializes a bundle of patches from given binary stream in ::DeserializePatch format.
class PatchBundleDeserializer final
{
public:
    /// \brief Registers given type as possible type for patches inside bundles.
    void RegisterType (const StandardLayout::Mapping &_mapping) noexcept;

    /// \brief Starts new bundle deserialization session.
    /// \invariant Input stream existence must be guaranteed by user until session is finished.
    void Begin (std::istream &_input) noexcept;

    /// \return Whether bundle has more unread patches.
    [[nodiscard]] bool HasNext () const noexcept;

    /// \brief Deserializes next patch from binary input stream.
    /// \return Next patch or `nullopt` if error happened.
    Container::Optional<StandardLayout::Patch> Next () noexcept;

    /// \brief Ends current bundle deserialization session.
    /// \details Safely processes case when there is no active deserialization session.
    void End () noexcept;

private:
    std::istream *input = nullptr;

    StandardLayout::PatchBuilder patchBuilder;

    Container::HashMap<Memory::UniqueString, StandardLayout::Mapping> typeRegister {
        Memory::Profiler::AllocationGroup {Memory::UniqueString {"PatchBundleDeserializer"}}};
};
} // namespace Emergence::Serialization::Binary

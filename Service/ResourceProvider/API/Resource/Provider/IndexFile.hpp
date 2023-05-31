#pragma once

#include <cstdint>

#include <Container/String.hpp>
#include <Container/Vector.hpp>

#include <StandardLayout/Mapping.hpp>

namespace Emergence::Resource::Provider
{
/// \brief Describes indexed resource which structure is described using reflection.
struct IndexFileObjectItem final
{
    /// \brief Object unique id.
    Memory::UniqueString id;

    /// \brief Name of object type.
    Memory::UniqueString typeName;

    /// \brief Path to resource from source root.
    Container::Utf8String relativePath;

    struct Reflection final
    {
        StandardLayout::FieldId id;
        StandardLayout::FieldId typeName;
        StandardLayout::FieldId relativePath;
        StandardLayout::Mapping mapping;
    };

    static const Reflection &Reflect () noexcept;
};

/// \brief Describes indexed resource of third party (without reflection) format.
struct IndexFileThirdPartyItem final
{
    /// \brief Resource unique id.
    Memory::UniqueString id;

    /// \brief Path to resource from source root.
    Container::Utf8String relativePath;

    struct Reflection final
    {
        StandardLayout::FieldId id;
        StandardLayout::FieldId relativePath;
        StandardLayout::Mapping mapping;
    };

    static const Reflection &Reflect () noexcept;
};

/// \brief Describes structure of the resource provider source index file.
/// \details Index files are used to cache information about source files and avoid scanning the filesystem.
struct IndexFile final
{
    /// \brief Resource provider expects index file to be named this way.
    inline static const Container::Utf8String INDEX_FILE_NAME = ".resource.provider.index";

    /// \brief List of all resources backed by reflection.
    Container::Vector<IndexFileObjectItem> objects {
        Memory::Profiler::AllocationGroup {Memory::UniqueString {"IndexFile"}}};

    /// \brief List of all resources with third party (not backed by reflection) formats.
    Container::Vector<IndexFileThirdPartyItem> thirdParty {
        Memory::Profiler::AllocationGroup {Memory::UniqueString {"IndexFile"}}};

    struct Reflection final
    {
        StandardLayout::FieldId objects;
        StandardLayout::FieldId thirdParty;
        StandardLayout::Mapping mapping;
    };

    static const Reflection &Reflect () noexcept;
};
} // namespace Emergence::Resource::Provider

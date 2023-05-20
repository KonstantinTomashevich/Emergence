#pragma once

#include <cstdint>

#include <Container/String.hpp>
#include <Container/Vector.hpp>

#include <StandardLayout/Mapping.hpp>

namespace Emergence::ResourceProvider
{
struct IndexFileObjectItem final
{
    Memory::UniqueString id;

    Memory::UniqueString typeName;

    Container::String relativePath;

    struct Reflection final
    {
        StandardLayout::FieldId id;
        StandardLayout::FieldId typeName;
        StandardLayout::FieldId relativePath;
        StandardLayout::Mapping mapping;
    };

    static const Reflection &Reflect () noexcept;
};

struct IndexFileThirdPartyItem final
{
    Memory::UniqueString id;

    Container::String relativePath;

    struct Reflection final
    {
        StandardLayout::FieldId id;
        StandardLayout::FieldId relativePath;
        StandardLayout::Mapping mapping;
    };

    static const Reflection &Reflect () noexcept;
};

struct IndexFile final
{
    inline static const Container::String INDEX_FILE_NAME = ".resource.provider.index";

    Container::Vector<IndexFileObjectItem> objects {
        Memory::Profiler::AllocationGroup {Memory::UniqueString {"IndexFile"}}};

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
} // namespace Emergence::ResourceProvider

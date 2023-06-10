#pragma once

#include <Container/String.hpp>
#include <Container/Vector.hpp>

#include <StandardLayout/Mapping.hpp>

namespace Emergence::VirtualFileSystem
{
enum class MountSource : uint8_t
{
    FILESYSTEM = 0u,
    PACKAGE
};

struct MountConfiguration final
{
    MountSource source = MountSource::FILESYSTEM;

    Container::Utf8String sourcePath;

    Container::Utf8String targetPath;

    struct Reflection final
    {
        StandardLayout::FieldId source;
        StandardLayout::FieldId sourcePath;
        StandardLayout::FieldId targetPath;
        StandardLayout::Mapping mapping;
    };

    static const Reflection &Reflect () noexcept;
};

struct MountConfigurationList final
{
    Container::Vector<MountConfiguration> items {
        Memory::Profiler::AllocationGroup {Memory::UniqueString {"MountConfigurationList"}}};

    struct Reflection final
    {
        StandardLayout::FieldId items;
        StandardLayout::Mapping mapping;
    };

    static const Reflection &Reflect () noexcept;
};
} // namespace Emergence::VirtualFileSystem

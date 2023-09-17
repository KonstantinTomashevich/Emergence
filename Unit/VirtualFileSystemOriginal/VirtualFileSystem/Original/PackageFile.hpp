#pragma once

#include <cstdint>

#include <Container/String.hpp>
#include <Container/Vector.hpp>

#include <StandardLayout/Mapping.hpp>

namespace Emergence::VirtualFileSystem::Original
{
struct PackageHeaderEntry final
{
    Container::Utf8String relativePath;

    /// \details For easier patch building, offset is given after header (not from the start of the file).
    ///          Full file offset can be easily calculated after reading the whole header.
    std::uint64_t offset;

    std::uint64_t size;

    struct Reflection final
    {
        StandardLayout::FieldId relativePath;
        StandardLayout::FieldId offset;
        StandardLayout::FieldId size;
        StandardLayout::Mapping mapping;
    };

    static const Reflection &Reflect () noexcept;
};

struct PackageHeader final
{
    Container::Vector<PackageHeaderEntry> entries {
        Memory::Profiler::AllocationGroup {Memory::UniqueString {"VirtualFileSystemPackageHeader"}}};

    struct Reflection final
    {
        StandardLayout::FieldId entries;
        StandardLayout::Mapping mapping;
    };

    static const Reflection &Reflect () noexcept;
};
} // namespace Emergence::VirtualFileSystem::Original

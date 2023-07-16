#pragma once

#include <Container/String.hpp>
#include <Container/Vector.hpp>

#include <StandardLayout/Mapping.hpp>

namespace Emergence::VirtualFileSystem
{
/// \brief Describes the source of data for mounting it into virtual file system.
enum class MountSource : uint8_t
{
    /// \brief Source is real file system path.
    FILE_SYSTEM = 0u,

    /// \brief Source is read-only package that lies under given real file system path.
    PACKAGE
};

/// \brief Describes parameters for single mount operation.
struct MountConfiguration final
{
    /// \brief Describes the source of data for mounting it into virtual file system.
    MountSource source = MountSource::FILE_SYSTEM;

    /// \brief Real file system path to source.
    Container::Utf8String sourcePath;

    /// \brief Mounting target path in virtual file system that is relative to mount root.
    Container::Utf8String targetPath;

    [[nodiscard]] bool operator== (const MountConfiguration &_other) const noexcept = default;

    [[nodiscard]] bool operator!= (const MountConfiguration &_other) const noexcept = default;

    struct Reflection final
    {
        StandardLayout::FieldId source;
        StandardLayout::FieldId sourcePath;
        StandardLayout::FieldId targetPath;
        StandardLayout::Mapping mapping;
    };

    static const Reflection &Reflect () noexcept;
};

/// \brief Convenience structure for storing multiple mount configurations as file on drive.
struct MountConfigurationList final
{
    Container::Vector<MountConfiguration> items {
        Memory::Profiler::AllocationGroup {Memory::UniqueString {"MountConfigurationList"}}};

    [[nodiscard]] bool operator== (const MountConfigurationList &_other) const noexcept = default;

    [[nodiscard]] bool operator!= (const MountConfigurationList &_other) const noexcept = default;

    struct Reflection final
    {
        StandardLayout::FieldId items;
        StandardLayout::Mapping mapping;
    };

    static const Reflection &Reflect () noexcept;
};
} // namespace Emergence::VirtualFileSystem

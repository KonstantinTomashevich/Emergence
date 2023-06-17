#pragma once

#include <API/Common/ImplementationBinding.hpp>

#include <Container/String.hpp>

#include <VirtualFileSystem/Entry.hpp>
#include <VirtualFileSystem/MountConfiguration.hpp>

namespace Emergence::VirtualFileSystem
{
constexpr const char PATH_SEPARATOR = '/';

class Context final
{
public:
    Context () noexcept;

    Context (const Context &_context) = delete;

    Context (Context &&_context) noexcept;

    ~Context () noexcept;

    EMERGENCE_DELETE_ASSIGNMENT (Context);

    [[nodiscard]] Entry GetRoot () const noexcept;

    Entry CreateFile (Entry _parent, const std::string_view &_fileName) noexcept;

    Entry CreateDirectory (Entry _parent, const std::string_view &_directoryName) noexcept;

    Entry MakeDirectories (const std::string_view &_absolutePath) noexcept;

    Entry MakeDirectories (Entry _parent, const std::string_view &_relativePath) noexcept;

    bool Delete (const Entry &_entry, bool _recursive, bool _includingFileSystem) noexcept;

    bool Mount (const Entry &_at, const MountConfiguration &_configuration) noexcept;

private:
    friend class Entry;
    friend class PackageBuilder;

    EMERGENCE_BIND_IMPLEMENTATION_INPLACE (sizeof (std::uint64_t) * 2u);
};
} // namespace Emergence::VirtualFileSystem

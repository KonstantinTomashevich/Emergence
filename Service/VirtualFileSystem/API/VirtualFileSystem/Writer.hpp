#pragma once

#include <cstdint>
#include <iostream>

#include <API/Common/ImplementationBinding.hpp>
#include <API/Common/Shortcuts.hpp>

#include <VirtualFileSystem/Entry.hpp>
#include <VirtualFileSystem/OpenMode.hpp>

namespace Emergence::VirtualFileSystem
{
/// \brief Provides API for writing files through virtual file system.
class Writer final
{
public:
    /// \brief Constructs writer for given file entry with given open mode.
    /// \warning Current content of the file will be destroyed by this operation!
    Writer (const Entry &_entry, OpenMode _openMode) noexcept;

    Writer (const Writer &_other) = delete;

    Writer (Writer &&_other) = delete;

    ~Writer () noexcept;

    /// \return Whether writer is in valid state.
    [[nodiscard]] bool IsValid () const noexcept;

    /// \brief Standard output stream for writing file data.
    std::ostream &OutputStream () noexcept;

    inline explicit operator bool () const noexcept
    {
        return IsValid ();
    }

    EMERGENCE_DELETE_ASSIGNMENT (Writer);

private:
    EMERGENCE_BIND_IMPLEMENTATION_INPLACE (sizeof (std::uintptr_t) * 172u);
};
} // namespace Emergence::VirtualFileSystem

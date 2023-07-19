#pragma once

#include <VirtualFileSystemApi.hpp>

#include <cstdint>
#include <iostream>

#include <API/Common/ImplementationBinding.hpp>
#include <API/Common/Shortcuts.hpp>

namespace Emergence::VirtualFileSystem
{
/// \brief Provides API for reading files through virtual file system.
class VirtualFileSystemApi Reader final
{
public:
    /// \brief Constructs reader for given file entry with given open mode.
    /// \warning Currently, all files are opened in binary mode only!
    Reader (const Entry &_entry) noexcept;

    Reader (const Reader &_other) = delete;

    Reader (Reader &&_other) = delete;

    ~Reader () noexcept;

    /// \return Whether reader is in valid state.
    [[nodiscard]] bool IsValid () const noexcept;

    /// \brief Standard input stream for reading file data.
    std::istream &InputStream () noexcept;

    inline explicit operator bool () const noexcept
    {
        return IsValid ();
    }

    EMERGENCE_DELETE_ASSIGNMENT (Reader);

private:
    EMERGENCE_BIND_IMPLEMENTATION_INPLACE (sizeof (std::uint64_t) * 175u);
};
} // namespace Emergence::VirtualFileSystem

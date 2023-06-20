#pragma once

#include <cstdint>
#include <iostream>

#include <API/Common/ImplementationBinding.hpp>
#include <API/Common/Shortcuts.hpp>

#include <VirtualFileSystem/FilePivot.hpp>
#include <VirtualFileSystem/OpenMode.hpp>

namespace Emergence::VirtualFileSystem
{
class Reader final
{
public:
    Reader (const Entry &_entry, OpenMode _openMode) noexcept;

    Reader (const Reader &_other) = delete;

    Reader (Reader &&_other) = delete;

    ~Reader () noexcept;

    [[nodiscard]] bool IsValid () const noexcept;

    std::istream &InputStream () noexcept;

    inline explicit operator bool () const noexcept
    {
        return IsValid ();
    }

    EMERGENCE_DELETE_ASSIGNMENT (Reader);

private:
    EMERGENCE_BIND_IMPLEMENTATION_INPLACE (sizeof (std::uint64_t) * 160u);
};
} // namespace Emergence::VirtualFileSystem

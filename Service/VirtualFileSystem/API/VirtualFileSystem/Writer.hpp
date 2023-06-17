#pragma once

#include <cstdint>
#include <iostream>

#include <API/Common/ImplementationBinding.hpp>
#include <API/Common/Shortcuts.hpp>

#include <VirtualFileSystem/Entry.hpp>
#include <VirtualFileSystem/FilePivot.hpp>
#include <VirtualFileSystem/OpenMode.hpp>

namespace Emergence::VirtualFileSystem
{
class Writer final
{
public:
    Writer (const Entry &_entry, OpenMode _openMode) noexcept;

    Writer (const Writer &_other) = delete;

    Writer (Writer &&_other) = delete;

    ~Writer () noexcept;

    [[nodiscard]] bool IsValid () const noexcept;

    std::ostream &OutputStream () noexcept;

    inline explicit operator bool () const noexcept
    {
        return IsValid ();
    }

    EMERGENCE_DELETE_ASSIGNMENT (Writer);

private:
    EMERGENCE_BIND_IMPLEMENTATION_INPLACE (sizeof (std::uintptr_t) * 33u);
};
} // namespace Emergence::VirtualFileSystem

#pragma once

#include <cstdint>

#include <API/Common/ImplementationBinding.hpp>
#include <API/Common/Shortcuts.hpp>

#include <VirtualFileSystem/FilePivot.hpp>
#include <VirtualFileSystem/OpenMode.hpp>

namespace Emergence::VirtualFileSystem
{
class Writer final
{
public:
    Writer (const Entry &_entry, OpenMode _openMode) noexcept;

    Writer (const Writer &_reader) = delete;

    Writer (Writer &&_reader) noexcept;

    ~Writer () noexcept;

    [[nodiscard]] bool IsValid () const noexcept;

    void WriteOne (std::uint8_t _byte) noexcept;

    void Write (std::uint8_t *_data, std::uint64_t _count) noexcept;

    EMERGENCE_DELETE_ASSIGNMENT (Writer);

private:
    EMERGENCE_BIND_IMPLEMENTATION_INPLACE (sizeof (std::uintptr_t) * 2u);
};
} // namespace Emergence::VirtualFileSystem

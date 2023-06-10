#pragma once

#include <cstdint>

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

    Reader (const Reader &_reader) = delete;

    Reader (Reader &&_reader) noexcept;

    ~Reader () noexcept;

    [[nodiscard]] bool IsValid () const noexcept;

    bool SeekPosition (FilePivot _pivot, std::uint64_t _position) noexcept;

    [[nodiscard]] std::uint64_t TellPosition () const noexcept;

    bool ReadOne (std::uint8_t &_output) noexcept;

    bool Read (std::uint8_t *_output, std::uint64_t _count) noexcept;

    EMERGENCE_DELETE_ASSIGNMENT (Reader);

private:
    EMERGENCE_BIND_IMPLEMENTATION_INPLACE (sizeof (std::uintptr_t) * 2u);
};
} // namespace Emergence::VirtualFileSystem

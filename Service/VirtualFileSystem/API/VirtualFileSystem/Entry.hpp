#pragma once

#include <cstdint>

#include <API/Common/Cursor.hpp>
#include <API/Common/ImplementationBinding.hpp>

#include <Container/String.hpp>

namespace Emergence::VirtualFileSystem
{
class Context;

enum class EntryType
{
    INVALID = 0u,
    FILE,
    DIRECTORY,
};

class Entry final
{
public:
    class Cursor final
    {
    public:
        Cursor (const Cursor &_other) noexcept;

        Cursor (Cursor &&_other) noexcept;

        ~Cursor () noexcept;

        [[nodiscard]] Entry operator* () const noexcept;

        Cursor &operator++ () noexcept;

        EMERGENCE_DELETE_ASSIGNMENT (Cursor);

    private:
        friend class Entry;

        EMERGENCE_BIND_IMPLEMENTATION_INPLACE (sizeof (std::uint64_t) * 9u);

        explicit Cursor (std::array<std::uint8_t, DATA_MAX_SIZE> &_data) noexcept;
    };

    Entry () noexcept;

    Entry (const Context &_context, const std::string_view &_absolutePath) noexcept;

    Entry (const Entry &_parent, const std::string_view &_relativePath) noexcept;

    Entry (const Entry &_entry) noexcept;

    Entry (Entry &&_entry) noexcept;

    ~Entry () noexcept;

    [[nodiscard]] EntryType GetType () const noexcept;

    [[nodiscard]] Container::Utf8String GetFileName () const noexcept;

    [[nodiscard]] Container::Utf8String GetExtension () const noexcept;

    [[nodiscard]] Container::Utf8String GetFullFileName () const noexcept;

    [[nodiscard]] Container::Utf8String GetFullPath () const noexcept;

    [[nodiscard]] Cursor ReadChildren () const noexcept;

    inline explicit operator bool () const noexcept
    {
        return GetType () != EntryType::INVALID;
    }

    Entry &operator= (const Entry &_other) noexcept;

    Entry &operator= (Entry &&_other) noexcept;

private:
    friend class Context;
    friend class Reader;
    friend class Writer;

    EMERGENCE_BIND_IMPLEMENTATION_INPLACE (sizeof (std::uint64_t) * 7u);

    Entry (std::array<std::uint8_t, DATA_MAX_SIZE> &_data) noexcept;
};
} // namespace Emergence::VirtualFileSystem

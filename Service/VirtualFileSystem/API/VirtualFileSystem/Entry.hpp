#pragma once

#include <cstdint>

#include <API/Common/ImplementationBinding.hpp>
#include <API/Common/Iterator.hpp>

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
    class Iterator final
    {
    public:
        EMERGENCE_FORWARD_ITERATOR_OPERATIONS (Iterator, Entry);

    private:
        friend class Entry;

        EMERGENCE_BIND_IMPLEMENTATION_INPLACE (sizeof (std::uintptr_t) * 4u);

        explicit Iterator (std::array<std::uint8_t, DATA_MAX_SIZE> &_data) noexcept;
    };

    Entry (const Context &_context, const char *_absolutePath) noexcept;

    Entry (const Entry &_parent, const char *_relativePath) noexcept;

    Entry (const Entry &_entry) noexcept;

    Entry (Entry &&_entry) noexcept;

    ~Entry () noexcept;

    [[nodiscard]] EntryType GetType () const noexcept;

    [[nodiscard]] Container::Utf8String GetFileName () const noexcept;

    [[nodiscard]] Container::Utf8String GetExtension () const noexcept;

    [[nodiscard]] Container::Utf8String GetFullFileName () const noexcept;

    [[nodiscard]] Container::Utf8String GetFullPath () const noexcept;

    [[nodiscard]] Iterator IterateOverChildren () const noexcept;

    Entry &operator= (const Entry &_entry) noexcept;

    Entry &operator= (Entry &&_entry) noexcept;

private:
    EMERGENCE_BIND_IMPLEMENTATION_INPLACE (sizeof (std::uint64_t));
};
} // namespace Emergence::VirtualFileSystem

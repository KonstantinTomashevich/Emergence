#pragma once

#include <VirtualFileSystemApi.hpp>

#include <chrono>
#include <cstdint>

#include <API/Common/Cursor.hpp>
#include <API/Common/ImplementationBinding.hpp>

#include <Container/String.hpp>

namespace Emergence::VirtualFileSystem
{
class Context;

/// \brief Represents type of virtual file system entry.
enum class EntryType
{
    /// \brief It is not possible to map entry to existing entity in virtual or real file system.
    INVALID = 0u,

    /// \brief Entry points to existing file: either virtual (from read-only package) or real one.
    FILE,

    /// \brief Entry points to existing directory: either virtual or real one.
    DIRECTORY,
};

/// \brief Weak pointer to virtual file system entry: file or directory.
class VirtualFileSystemApi Entry final
{
public:
    /// \brief Provides API for iterating over entry children if entry is directory.
    class VirtualFileSystemApi Cursor final
    {
    public:
        Cursor (const Cursor &_other) noexcept;

        Cursor (Cursor &&_other) noexcept;

        ~Cursor () noexcept;

        /// \return Entry that points to current child or invalid entry if there is no more children.
        [[nodiscard]] Entry operator* () const noexcept;

        /// \brief Moves to next child.
        /// \invariant Cursor is not empty (does not return invalid entry).
        Cursor &operator++ () noexcept;

        EMERGENCE_DELETE_ASSIGNMENT (Cursor);

    private:
        friend class Entry;

        EMERGENCE_BIND_IMPLEMENTATION_INPLACE (sizeof (std::uint64_t) * 9u);

        explicit Cursor (std::array<std::uint8_t, DATA_MAX_SIZE> &_data) noexcept;
    };

    /// \brief Constructs invalid entry.
    Entry () noexcept;

    /// \brief Constructs entry that points to object under given absolute path.
    ///        If object does not exists, entry is invalid.
    Entry (const Context &_context, const std::string_view &_absolutePath) noexcept;

    /// \brief Constructs entry that points to object under given relative path.
    ///        If object does not exists, entry is invalid.
    Entry (const Entry &_parent, const std::string_view &_relativePath) noexcept;

    Entry (const Entry &_entry) noexcept;

    Entry (Entry &&_entry) noexcept;

    ~Entry () noexcept;

    /// \return Detects up-to-date type of this entry.
    /// \details If object to which entry points was deleted, type will be automatically changed to invalid.
    [[nodiscard]] EntryType GetType () const noexcept;

    /// \return File name without last extension.
    /// \invariant Entry is valid.
    [[nodiscard]] Container::Utf8String GetName () const noexcept;

    /// \return Last extension.
    /// \invariant Entry is valid.
    [[nodiscard]] Container::Utf8String GetExtension () const noexcept;

    /// \return File name including last extension.
    /// \invariant Entry is valid.
    [[nodiscard]] Container::Utf8String GetFullName () const noexcept;

    /// \return Absolute object path in virtual file system.
    /// \invariant Entry is valid.
    [[nodiscard]] Container::Utf8String GetFullPath () const noexcept;

    /// \return Time point at which file was last written to.
    /// \invariant Entry is file.
    [[nodiscard]] std::chrono::time_point<std::chrono::file_clock> GetLastWriteTime () const noexcept;

    /// \return Cursor for iteration over entry children.
    /// \invariant Entry is directory.
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

#include <Assert/Assert.hpp>

#include <SyntaxSugar/BlockCast.hpp>

#include <VirtualFileSystem/Entry.hpp>
#include <VirtualFileSystem/Original/Core.hpp>
#include <VirtualFileSystem/Original/Wrappers.hpp>

namespace Emergence::VirtualFileSystem
{
Entry::Cursor::Cursor (const Entry::Cursor &_other) noexcept
{
    new (&data) Original::CursorImplementationData {block_cast<Original::CursorImplementationData> (_other.data)};
}

Entry::Cursor::Cursor (Emergence::VirtualFileSystem::Entry::Cursor &&_other) noexcept
{
    new (&data)
        Original::CursorImplementationData {std::move (block_cast<Original::CursorImplementationData> (_other.data))};
}

Entry::Cursor::~Cursor () noexcept
{
    block_cast<Original::CursorImplementationData> (data).~CursorImplementationData ();
}

Entry Entry::Cursor::operator* () const noexcept
{
    const auto &implementationData = block_cast<Original::CursorImplementationData> (data);
    Original::EntryImplementationData entry {implementationData.owner, *implementationData.cursor};
    return {array_cast (entry)};
}

Entry::Cursor &Entry::Cursor::operator++ () noexcept
{
    ++block_cast<Original::CursorImplementationData> (data).cursor;
    return *this;
}

Entry::Cursor::Cursor (std::array<std::uint8_t, DATA_MAX_SIZE> &_data) noexcept
{
    new (&data) Original::CursorImplementationData {std::move (block_cast<Original::CursorImplementationData> (_data))};
}

Entry::Entry () noexcept
{
    new (&data) Original::EntryImplementationData {nullptr, {}};
}

Entry::Entry (const Context &_context, const std::string_view &_absolutePath) noexcept
    : Entry (_context.GetRoot (), _absolutePath)
{
}

Entry::Entry (const Entry &_parent, const std::string_view &_relativePath) noexcept
{
    const auto &parentData = block_cast<Original::EntryImplementationData> (_parent.data);
    EMERGENCE_ASSERT (parentData.owner);
    new (&data) Original::EntryImplementationData {parentData.owner,
                                                   parentData.owner->Resolve (parentData.object, _relativePath)};
}

Entry::Entry (const Entry &_entry) noexcept
{
    new (&data) Original::EntryImplementationData {block_cast<Original::EntryImplementationData> (_entry.data)};
}

Entry::Entry (Entry &&_entry) noexcept
{
    new (&data)
        Original::EntryImplementationData {std::move (block_cast<Original::EntryImplementationData> (_entry.data))};
    block_cast<Original::EntryImplementationData> (_entry.data).owner = nullptr;
}

Entry::~Entry () noexcept
{
    block_cast<Original::EntryImplementationData> (data).~EntryImplementationData ();
}

EntryType Entry::GetType () const noexcept
{
    const auto &entryData = block_cast<Original::EntryImplementationData> (data);
    if (!entryData.owner)
    {
        return EntryType::INVALID;
    }

    switch (entryData.object.type)
    {
    case Original::ObjectType::INVALID:
        return EntryType::INVALID;

    case Original::ObjectType::ENTRY:
        switch (entryData.owner->GetEntryType (entryData.object.entryId))
        {
        case Original::EntryType::VIRTUAL_DIRECTORY:
        case Original::EntryType::FILE_SYSTEM_LINK:
            return EntryType::DIRECTORY;

        case Original::EntryType::PACKAGE_FILE:
            return EntryType::FILE;
        }

        break;

    case Original::ObjectType::PATH:
        switch (std::filesystem::status (entryData.object.path).type ())
        {
        case std::filesystem::file_type::none:
        case std::filesystem::file_type::not_found:
        case std::filesystem::file_type::symlink:
        case std::filesystem::file_type::block:
        case std::filesystem::file_type::character:
        case std::filesystem::file_type::fifo:
        case std::filesystem::file_type::socket:
        case std::filesystem::file_type::unknown:
        case std::filesystem::file_type::junction:
            return EntryType::INVALID;

        case std::filesystem::file_type::regular:
            return EntryType::FILE;

        case std::filesystem::file_type::directory:
            return EntryType::DIRECTORY;
        }

        break;
    }

    EMERGENCE_ASSERT (false);
    return EntryType::INVALID;
}

Container::Utf8String Entry::GetFileName () const noexcept
{
    Container::Utf8String fullName = GetFullFileName ();
    const std::size_t dotPos = fullName.find_last_of ('.');

    if (dotPos == std::string::npos)
    {
        return fullName;
    }

    return fullName.substr (0u, dotPos);
}

Container::Utf8String Entry::GetExtension () const noexcept
{
    Container::Utf8String fullName = GetFullFileName ();
    std::size_t dotPos = fullName.find_last_of ('.');

    if (dotPos == std::string::npos)
    {
        return "";
    }

    return fullName.substr (dotPos, std::string::npos);
}

Container::Utf8String Entry::GetFullFileName () const noexcept
{
    const auto &entryData = block_cast<Original::EntryImplementationData> (data);
    switch (entryData.object.type)
    {
    case Original::ObjectType::INVALID:
        EMERGENCE_ASSERT (false);
        return {};

    case Original::ObjectType::ENTRY:
        return *entryData.owner->GetEntryName (entryData.object.entryId);

    case Original::ObjectType::PATH:
        return std::filesystem::path (entryData.object.path)
            .filename ()
            .generic_string<char, std::char_traits<char>, Memory::HeapSTD<char>> ();
    }

    EMERGENCE_ASSERT (false);
    return {};
}

Container::Utf8String Entry::GetFullPath () const noexcept
{
    const auto &entryData = block_cast<Original::EntryImplementationData> (data);
    return entryData.owner->ExtractFullVirtualPath (entryData.object);
}

Entry::Cursor Entry::ReadChildren () const noexcept
{
    const auto &entryData = block_cast<Original::EntryImplementationData> (data);
    Original::CursorImplementationData cursorData {entryData.owner, {entryData.owner, entryData.object}};
    return Entry::Cursor {array_cast (cursorData)};
}

Entry &Entry::operator= (const Entry &_other) noexcept
{
    if (this != &_other)
    {
        this->~Entry ();
        new (this) Entry (_other);
    }

    return *this;
}

Entry &Entry::operator= (Entry &&_other) noexcept
{
    if (this != &_other)
    {
        this->~Entry ();
        new (this) Entry (std::move (_other));
    }

    return *this;
}

Entry::Entry (std::array<std::uint8_t, DATA_MAX_SIZE> &_data) noexcept
{
    new (&data) Original::EntryImplementationData {std::move (block_cast<Original::EntryImplementationData> (_data))};
}
} // namespace Emergence::VirtualFileSystem

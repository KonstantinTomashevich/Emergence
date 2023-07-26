#include <filesystem>

#include <API/Common/BlockCast.hpp>

#include <Assert/Assert.hpp>

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

static EntryType QueryType (Original::VirtualFileSystem *_owner, const Original::Object &_object) noexcept
{
    if (!_owner)
    {
        return EntryType::INVALID;
    }

    switch (_object.type)
    {
    case Original::ObjectType::INVALID:
        return EntryType::INVALID;

    case Original::ObjectType::ENTRY:
        switch (_owner->GetEntryType (_object.entryId))
        {
        case Original::EntryType::VIRTUAL_DIRECTORY:
        case Original::EntryType::FILE_SYSTEM_LINK:
            return EntryType::DIRECTORY;

        case Original::EntryType::VIRTUAL_FILE:
        case Original::EntryType::PACKAGE_FILE:
            return EntryType::FILE;

        case Original::EntryType::WEAK_FILE_LINK:
            return QueryType (_owner, _owner->GetWeakFileLinkTarget (_object.entryId));
        }

        break;

    case Original::ObjectType::PATH:
        switch (std::filesystem::status (_object.path).type ())
        {
        case std::filesystem::file_type::regular:
            return EntryType::FILE;

        case std::filesystem::file_type::directory:
            return EntryType::DIRECTORY;

        // Unfortunately, we need to use default here for better support across
        // different standards: not all versions of STL support all the entry types.
        default:
            return EntryType::INVALID;
        }
    }

    EMERGENCE_ASSERT (false);
    return EntryType::INVALID;
}

EntryType Entry::GetType () const noexcept
{
    const auto &entryData = block_cast<Original::EntryImplementationData> (data);
    return QueryType (entryData.owner, entryData.object);
}

Container::Utf8String Entry::GetName () const noexcept
{
    Container::Utf8String fullName = GetFullName ();
    const std::size_t dotPos = fullName.find_last_of ('.');

    if (dotPos == std::string::npos)
    {
        return fullName;
    }

    return fullName.substr (0u, dotPos);
}

Container::Utf8String Entry::GetExtension () const noexcept
{
    Container::Utf8String fullName = GetFullName ();
    std::size_t dotPos = fullName.find_last_of ('.');

    if (dotPos == std::string::npos)
    {
        return "";
    }

    return fullName.substr (dotPos + 1u, std::string::npos);
}

Container::Utf8String Entry::GetFullName () const noexcept
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

static std::chrono::time_point<std::chrono::file_clock> QueryLastWriteTime (Original::VirtualFileSystem *_owner,
                                                                            const Original::Object &_object) noexcept
{
    if (!_owner)
    {
        return {};
    }

    switch (_object.type)
    {
    case Original::ObjectType::INVALID:
        return {};

    case Original::ObjectType::ENTRY:
        switch (_owner->GetEntryType (_object.entryId))
        {
        case Original::EntryType::VIRTUAL_DIRECTORY:
        case Original::EntryType::FILE_SYSTEM_LINK:
            return {};

        case Original::EntryType::VIRTUAL_FILE:
            return _owner->GetVirtualFileLastWriteTime (_object.entryId);

        case Original::EntryType::PACKAGE_FILE:
            return std::filesystem::last_write_time (_owner->GetPackageFilePath (_object.entryId));

        case Original::EntryType::WEAK_FILE_LINK:
            return QueryLastWriteTime (_owner, _owner->GetWeakFileLinkTarget (_object.entryId));
        }

        break;

    case Original::ObjectType::PATH:
        switch (std::filesystem::status (_object.path).type ())
        {
        case std::filesystem::file_type::regular:
            return std::filesystem::last_write_time (_object.path);

        case std::filesystem::file_type::directory:
            EMERGENCE_ASSERT (false);
            return {};

        // Unfortunately, we need to use default here for better support across
        // different standards: not all versions of STL support all the entry types.
        default:
            return {};
        }
    }

    EMERGENCE_ASSERT (false);
    return {};
}

std::chrono::time_point<std::chrono::file_clock> Entry::GetLastWriteTime () const noexcept
{
    const auto &entryData = block_cast<Original::EntryImplementationData> (data);
    return QueryLastWriteTime (entryData.owner, entryData.object);
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

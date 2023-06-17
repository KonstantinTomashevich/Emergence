#pragma once

#include <atomic>
#include <cstdio>
#include <filesystem>
#include <limits>

#include <Container/String.hpp>

#include <Memory/UniqueString.hpp>

#include <RecordCollection/Collection.hpp>

#include <VirtualFileSystem/Context.hpp>
#include <VirtualFileSystem/MountConfiguration.hpp>
#include <VirtualFileSystem/OpenMode.hpp>

namespace Emergence::VirtualFileSystem::Original
{
using EntryId = std::uint64_t;

constexpr EntryId INVALID_ID = std::numeric_limits<EntryId>::max ();

constexpr EntryId ROOT_ID = 0u;

enum class EntryType : uint8_t
{
    VIRTUAL_DIRECTORY = 0u,
    PACKAGE_FILE,
    FILE_SYSTEM_LINK,
};

struct Entry final
{
    EntryId id = INVALID_ID;

    EntryId parentId = INVALID_ID;

    Memory::UniqueString name;

    EntryType type = EntryType::VIRTUAL_DIRECTORY;

    std::uint64_t packageFileOffset = 0u;

    std::uint64_t packageFileSize = 0u;

    Container::Utf8String filesystemLink;

    struct Reflection final
    {
        StandardLayout::FieldId id;
        StandardLayout::FieldId parentId;
        StandardLayout::FieldId name;
        StandardLayout::FieldId type;
        StandardLayout::FieldId packageFileOffset;
        StandardLayout::FieldId packageFileSize;
        StandardLayout::FieldId filesystemLink;
        StandardLayout::Mapping mapping;
    };

    static const Reflection &Reflect () noexcept;
};

enum class ObjectType : uint8_t
{
    INVALID = 0u,
    ENTRY,
    PATH,
};

struct Object final
{
    Object () noexcept;

    Object (EntryId _entryId) noexcept;

    Object (Container::Utf8String _path) noexcept;

    Object (const Object &_other) noexcept;

    Object (Object &&_other) noexcept;

    ~Object () noexcept;

    Object &operator= (const Object &_other) noexcept;

    Object &operator= (Object &&_other) noexcept;

    ObjectType type;

    union
    {
        EntryId entryId;

        Container::Utf8String path;
    };
};

struct FileReadContext final
{
    FILE *file = nullptr;
    std::uint64_t offset = 0u;
    std::uint64_t size = 0u;
};

struct FileWriteContext final
{
    FILE *file = nullptr;
};

class VirtualFileSystem
{
public:
    class Cursor final
    {
    public:
        Cursor (VirtualFileSystem *_owner, const Object &_source) noexcept;

        Cursor (const Cursor &_other) noexcept;

        Cursor (Cursor &&_other) noexcept;

        ~Cursor () noexcept;

        [[nodiscard]] Object operator* () const noexcept;

        Cursor &operator++ () noexcept;

        EMERGENCE_DELETE_ASSIGNMENT (Cursor);

    private:
        enum class Type
        {
            REAL,
            VIRTUAL
        };

        Type type = Type::REAL;
        union
        {
            std::filesystem::directory_iterator realIterator;
            RecordCollection::PointRepresentation::ReadCursor virtualCursor;
        };
    };

    VirtualFileSystem () noexcept;

    [[nodiscard]] Object Resolve (const Object &_relativeTo, const std::string_view &_path) const noexcept;

    [[nodiscard]] Object FindParent (const Object &_object) const noexcept;

    [[nodiscard]] Object FindChild (const Object &_object, const std::string_view &_name) const noexcept;

    [[nodiscard]] Container::Utf8String ExtractFullVirtualPath (const Object &_object) const noexcept;

    Object CreateFile (const Object &_parent, const std::string_view &_fileName) noexcept;

    Object CreateDirectory (const Object &_parent, const std::string_view &_directoryName) noexcept;

    Object MakeDirectories (const Object &_parent, const std::string_view &_relativePath) noexcept;

    bool Delete (const Object &_entry, bool _recursive, bool _includingFileSystem) noexcept;

    bool Mount (const Object &_at, const MountConfiguration &_configuration) noexcept;

    [[nodiscard]] EntryType GetEntryType (EntryId _id) const noexcept;

    [[nodiscard]] Memory::UniqueString GetEntryName (EntryId _id) const noexcept;

    [[nodiscard]] FileReadContext OpenFileForRead (const Object &_object, OpenMode _mode) const noexcept;

    [[nodiscard]] FileWriteContext OpenFileForWrite (const Object &_object, OpenMode _mode) const noexcept;

private:
    friend class Iterator;

    RecordCollection::Collection entries;
    mutable RecordCollection::PointRepresentation entriesById;
    mutable RecordCollection::PointRepresentation entriesByParentId;
    mutable RecordCollection::PointRepresentation entriesByParentIdAndName;
    mutable RecordCollection::SignalRepresentation fileSystemLinkEntries;

    EntryId nextEntryId = ROOT_ID + 1u;
};
} // namespace Emergence::VirtualFileSystem::Original

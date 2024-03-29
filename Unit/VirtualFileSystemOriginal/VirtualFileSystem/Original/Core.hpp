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

namespace Emergence::VirtualFileSystem::Original
{
using EntryId = std::uint64_t;

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

    struct Reflection final
    {
        StandardLayout::FieldId type;
        StandardLayout::FieldId entryId;
        StandardLayout::FieldId path;
        StandardLayout::Mapping mapping;
    };

    static const Reflection &Reflect () noexcept;
};

constexpr EntryId INVALID_ID = std::numeric_limits<EntryId>::max ();

constexpr EntryId ROOT_ID = 0u;

enum class EntryType : uint8_t
{
    VIRTUAL_DIRECTORY = 0u,
    VIRTUAL_FILE,
    PACKAGE_FILE,
    FILE_SYSTEM_LINK,
    WEAK_FILE_LINK,
};

struct VirtualFileChunk final
{
    VirtualFileChunk *next = nullptr;
    VirtualFileChunk *previous = nullptr;
    std::uint64_t size = 0u;
    std::uint64_t used = 0u;

#if defined(_MSVC_STL_VERSION)
    // For some reason updated MSVC compiler doesn't like this zero size array.
    std::uint8_t data[1u];
#else
    std::uint8_t data[0u];
#endif
};

static_assert (std::is_trivially_destructible_v<VirtualFileChunk>);

struct VirtualFileData final
{
    EMERGENCE_STATIONARY_DATA_TYPE (VirtualFileData);

    void Reset () noexcept;

    VirtualFileChunk *firstChunk = nullptr;
    VirtualFileChunk *lastChunk = nullptr;
    std::uint64_t size = 0u;

    Memory::Heap *chunkHeap = nullptr;

    std::chrono::time_point<std::chrono::file_clock> lastWriteTime =
        std::chrono::time_point<std::chrono::file_clock>::min ();

    struct Reflection final
    {
        StandardLayout::Mapping mapping;
    };

    static const Reflection &Reflect () noexcept;
};

struct PackageFileData final
{
    Container::Utf8String path;
    std::uint64_t offset = 0u;
    std::uint64_t size = 0u;

    struct Reflection final
    {
        StandardLayout::FieldId path;
        StandardLayout::FieldId offset;
        StandardLayout::FieldId size;
        StandardLayout::Mapping mapping;
    };

    static const Reflection &Reflect () noexcept;
};

struct Entry final
{
    EMERGENCE_STATIONARY_DATA_TYPE (Entry);

    EntryId id = INVALID_ID;

    EntryId parentId = INVALID_ID;

    Memory::UniqueString name;

    EntryType type = EntryType::VIRTUAL_DIRECTORY;

    union
    {
        /// \details Mutable because of IO write access that changes last
        ///          write time and chunks, but is guaranteed to be unique.
        mutable VirtualFileData virtualFile;

        PackageFileData packageFile;
        Container::Utf8String filesystemLink;
        Object weakFileLink;
    };

    struct Reflection final
    {
        StandardLayout::FieldId id;
        StandardLayout::FieldId parentId;
        StandardLayout::FieldId name;
        StandardLayout::FieldId type;
        StandardLayout::FieldId virtualFile;
        StandardLayout::FieldId packageFile;
        StandardLayout::FieldId filesystemLink;
        StandardLayout::FieldId weakFileLink;
        StandardLayout::Mapping mapping;
    };

    static const Reflection &Reflect () noexcept;
};

enum class FileIOContextType
{
    REAL_FILE,
    VIRTUAL_FILE
};

struct RealFileReadContext final
{
    FILE *file = nullptr;
    std::uint64_t offset = 0u;
    std::uint64_t size = 0u;
};

static_assert (std::is_trivially_destructible_v<RealFileReadContext>);

struct FileReadContext final
{
    FileIOContextType type = FileIOContextType::REAL_FILE;
    union
    {
        RealFileReadContext realFile {};
        VirtualFileData *virtualFile;
    };
};

struct FileWriteContext final
{
    FileIOContextType type = FileIOContextType::REAL_FILE;
    union
    {
        FILE *realFile = nullptr;
        VirtualFileData *virtualFile;
    };
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

    Object CreateWeakFileLink (const Object &_target,
                               const Object &_parent,
                               const std::string_view &_linkName) noexcept;

    Object MakeDirectories (const Object &_parent, const std::string_view &_relativePath) noexcept;

    bool Delete (const Object &_entry, bool _recursive, bool _includingFileSystem) noexcept;

    bool Mount (const Object &_at, const MountConfiguration &_configuration) noexcept;

    [[nodiscard]] EntryType GetEntryType (EntryId _id) const noexcept;

    [[nodiscard]] Memory::UniqueString GetEntryName (EntryId _id) const noexcept;

    [[nodiscard]] std::chrono::time_point<std::chrono::file_clock> GetVirtualFileLastWriteTime (
        EntryId _id) const noexcept;

    [[nodiscard]] Container::Utf8String GetPackageFilePath (EntryId _id) const noexcept;

    [[nodiscard]] Object GetWeakFileLinkTarget (EntryId _id) const noexcept;

    [[nodiscard]] FileReadContext OpenFileForRead (const Object &_object) const noexcept;

    [[nodiscard]] FileWriteContext OpenFileForWrite (const Object &_object) const noexcept;

private:
    friend class Iterator;

    RecordCollection::Collection entries;
    mutable RecordCollection::PointRepresentation entriesById;
    mutable RecordCollection::PointRepresentation entriesByParentId;
    mutable RecordCollection::PointRepresentation entriesByParentIdAndName;
    mutable RecordCollection::SignalRepresentation fileSystemLinkEntries;

    EntryId nextEntryId = ROOT_ID + 1u;
    Memory::Heap virtualFileChunkHeap;
};
} // namespace Emergence::VirtualFileSystem::Original

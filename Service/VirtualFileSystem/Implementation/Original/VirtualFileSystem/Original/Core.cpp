#define _CRT_SECURE_NO_WARNINGS

#include <filesystem>
#include <fstream>

#include <Assert/Assert.hpp>

#include <Log/Log.hpp>

#include <StandardLayout/MappingRegistration.hpp>

#include <Serialization/Binary.hpp>

#include <SyntaxSugar/BlockCast.hpp>

#include <VirtualFileSystem/Original/Core.hpp>
#include <VirtualFileSystem/Original/PackageFile.hpp>

namespace Emergence::VirtualFileSystem::Original
{
using namespace Memory::Literals;

Object::Object () noexcept
    : type (ObjectType::INVALID)
{
}

Object::Object (EntryId _entryId) noexcept
    : type (ObjectType::ENTRY),
      entryId (_entryId)
{
}

Object::Object (Container::Utf8String _path) noexcept
    : type (ObjectType::PATH),
      path (std::move (_path))
{
}

Object::Object (const Object &_other) noexcept
    : type (_other.type)
{
    switch (type)
    {
    case ObjectType::INVALID:
        break;

    case ObjectType::ENTRY:
        entryId = _other.entryId;
        break;

    case ObjectType::PATH:
        new (&path) Container::Utf8String {_other.path};
        break;
    }
}

Object::Object (Object &&_other) noexcept
    : type (_other.type)
{
    switch (type)
    {
    case ObjectType::INVALID:
        break;

    case ObjectType::ENTRY:
        entryId = _other.entryId;
        break;

    case ObjectType::PATH:
        new (&path) Container::Utf8String {std::move (_other.path)};
        _other.path.clear ();
        break;
    }

    _other.type = ObjectType::INVALID;
}

Object::~Object () noexcept
{
    switch (type)
    {
    case ObjectType::INVALID:
    case ObjectType::ENTRY:
        break;

    case ObjectType::PATH:
        path.~basic_string ();
        break;
    }
}

Object &Object::operator= (const Object &_other) noexcept
{
    if (this != &_other)
    {
        this->~Object ();
        new (this) Object (_other);
    }

    return *this;
}

Object &Object::operator= (Object &&_other) noexcept
{
    if (this != &_other)
    {
        this->~Object ();
        new (this) Object (std::move (_other));
    }

    return *this;
}

const Object::Reflection &Object::Reflect () noexcept
{
    static const Reflection reflection = [] ()
    {
        EMERGENCE_MAPPING_REGISTRATION_BEGIN (Object);
        EMERGENCE_MAPPING_REGISTER_REGULAR (type);

        EMERGENCE_MAPPING_UNION_VARIANT_BEGIN (type, 0u);
        EMERGENCE_MAPPING_REGISTER_REGULAR (entryId);
        EMERGENCE_MAPPING_UNION_VARIANT_END ();

        EMERGENCE_MAPPING_UNION_VARIANT_BEGIN (type, 0u);
        EMERGENCE_MAPPING_REGISTER_REGULAR (path);
        EMERGENCE_MAPPING_UNION_VARIANT_END ();

        EMERGENCE_MAPPING_REGISTRATION_END ();
    }();

    return reflection;
}

const PackageFileData::Reflection &PackageFileData::Reflect () noexcept
{
    static const Reflection reflection = [] ()
    {
        EMERGENCE_MAPPING_REGISTRATION_BEGIN (PackageFileData);
        EMERGENCE_MAPPING_REGISTER_REGULAR (path);
        EMERGENCE_MAPPING_REGISTER_REGULAR (offset);
        EMERGENCE_MAPPING_REGISTER_REGULAR (size);
        EMERGENCE_MAPPING_REGISTRATION_END ();
    }();

    return reflection;
}

// NOLINTNEXTLINE(modernize-use-equals-default): We cannot use default here due to union.
Entry::Entry () noexcept
{
}

Entry::~Entry () noexcept
{
    switch (type)
    {
    case EntryType::VIRTUAL_DIRECTORY:
        break;

    case EntryType::PACKAGE_FILE:
        packageFile.~PackageFileData ();
        break;

    case EntryType::FILE_SYSTEM_LINK:
        filesystemLink.~basic_string ();
        break;

    case EntryType::WEAK_FILE_LINK:
        weakFileLink.~Object ();
        break;
    }
}

const Entry::Reflection &Entry::Reflect () noexcept
{
    static const Reflection reflection = [] ()
    {
        EMERGENCE_MAPPING_REGISTRATION_BEGIN (Entry);
        EMERGENCE_MAPPING_REGISTER_REGULAR (id);
        EMERGENCE_MAPPING_REGISTER_REGULAR (parentId);
        EMERGENCE_MAPPING_REGISTER_REGULAR (name);
        EMERGENCE_MAPPING_REGISTER_REGULAR (type);

        EMERGENCE_MAPPING_UNION_VARIANT_BEGIN (type, 1u);
        EMERGENCE_MAPPING_REGISTER_REGULAR (packageFile);
        EMERGENCE_MAPPING_UNION_VARIANT_END ();

        EMERGENCE_MAPPING_UNION_VARIANT_BEGIN (type, 2u);
        EMERGENCE_MAPPING_REGISTER_REGULAR (filesystemLink);
        EMERGENCE_MAPPING_UNION_VARIANT_END ();

        EMERGENCE_MAPPING_UNION_VARIANT_BEGIN (type, 3u);
        EMERGENCE_MAPPING_REGISTER_REGULAR (weakFileLink);
        EMERGENCE_MAPPING_UNION_VARIANT_END ();

        EMERGENCE_MAPPING_REGISTRATION_END ();
    }();

    return reflection;
}

VirtualFileSystem::Cursor::Cursor (VirtualFileSystem *_owner, const Object &_source) noexcept
{
    switch (_source.type)
    {
    case ObjectType::INVALID:
        EMERGENCE_ASSERT (false);
        break;

    case ObjectType::ENTRY:
    {
        auto entryCursor = _owner->entriesById.ReadPoint (&_source.entryId);
        const auto *entry = static_cast<const Entry *> (*entryCursor);
        EMERGENCE_ASSERT (entry);

        switch (entry->type)
        {
        case EntryType::VIRTUAL_DIRECTORY:
            type = Type::VIRTUAL;
            new (&virtualCursor) RecordCollection::PointRepresentation::ReadCursor {
                _owner->entriesByParentId.ReadPoint (&_source.entryId)};
            break;

        case EntryType::PACKAGE_FILE:
        case EntryType::WEAK_FILE_LINK:
            EMERGENCE_ASSERT (false);
            break;

        case EntryType::FILE_SYSTEM_LINK:
            type = Type::REAL;
            new (&realIterator) std::filesystem::directory_iterator {entry->filesystemLink};
            break;
        }

        break;
    }

    case ObjectType::PATH:
        type = Type::REAL;
        new (&realIterator) std::filesystem::directory_iterator {_source.path};
        break;
    }
}

VirtualFileSystem::Cursor::Cursor (const VirtualFileSystem::Cursor &_other) noexcept
    : type (_other.type)
{
    switch (type)
    {
    case Type::REAL:
        new (&realIterator) std::filesystem::directory_iterator {_other.realIterator};
        break;

    case Type::VIRTUAL:
        new (&virtualCursor) RecordCollection::PointRepresentation::ReadCursor {_other.virtualCursor};
        break;
    }
}

VirtualFileSystem::Cursor::Cursor (VirtualFileSystem::Cursor &&_other) noexcept
    : type (_other.type)
{
    switch (type)
    {
    case Type::REAL:
        new (&realIterator) std::filesystem::directory_iterator {std::move (_other.realIterator)};
        break;

    case Type::VIRTUAL:
        new (&virtualCursor) RecordCollection::PointRepresentation::ReadCursor {std::move (_other.virtualCursor)};
        break;
    }
}

VirtualFileSystem::Cursor::~Cursor () noexcept
{
    switch (type)
    {
    case Type::REAL:
        realIterator.~directory_iterator ();
        break;

    case Type::VIRTUAL:
        virtualCursor.~ReadCursor ();
        break;
    }
}

Object VirtualFileSystem::Cursor::operator* () const noexcept
{
    switch (type)
    {
    case Type::REAL:
        if (realIterator != std::filesystem::end (realIterator))
        {
            return {realIterator->path ().generic_string<char, std::char_traits<char>, Memory::HeapSTD<char>> ()};
        }

        return {};

    case Type::VIRTUAL:
        if (const auto *entry = static_cast<const Entry *> (*virtualCursor))
        {
            return {entry->id};
        }

        return {};
    }

    EMERGENCE_ASSERT (false);
    return {};
}

VirtualFileSystem::Cursor &VirtualFileSystem::Cursor::operator++ () noexcept
{
    switch (type)
    {
    case Type::REAL:
        ++realIterator;
        break;

    case Type::VIRTUAL:
        ++virtualCursor;
        break;
    }

    return *this;
}

// Internal static, because should not normally be used by user: only appears as a result of full path usage.
static const char *ROOT_SELECTOR = "~";

VirtualFileSystem::VirtualFileSystem () noexcept
    : entries (Entry::Reflect ().mapping),
      entriesById (entries.CreatePointRepresentation ({Entry::Reflect ().id})),
      entriesByParentId (entries.CreatePointRepresentation ({Entry::Reflect ().parentId})),
      entriesByParentIdAndName (
          entries.CreatePointRepresentation ({Entry::Reflect ().parentId, Entry::Reflect ().name})),
      fileSystemLinkEntries (entries.CreateSignalRepresentation (
          Entry::Reflect ().type, array_cast<EntryType, sizeof (std::uint64_t)> (EntryType::FILE_SYSTEM_LINK)))
{
    auto inserter = entries.AllocateAndInsert ();
    auto *root = static_cast<Entry *> (inserter.Allocate ());
    root->id = ROOT_ID;
    root->parentId = INVALID_ID;
    root->name = Memory::UniqueString {ROOT_SELECTOR};
    root->type = EntryType::VIRTUAL_DIRECTORY;
}

Object VirtualFileSystem::Resolve (const Object &_relativeTo, const std::string_view &_path) const noexcept
{
    Object current = _relativeTo;
    if (current.type == ObjectType::INVALID)
    {
        EMERGENCE_LOG (ERROR, "VirtualFileSystem: Received resolve query with invalid base path!");
        return current;
    }

#if defined(EMERGENCE_ASSERT_ENABLED)
    if (current.type == ObjectType::ENTRY)
    {
        auto entryCursor = entriesById.ReadPoint (&current.entryId);
        EMERGENCE_ASSERT (*entryCursor);
    }
#endif

    auto currentStart = _path.begin ();
    auto iterator = _path.begin ();

    auto processPathStep = [this, &current, &currentStart, &iterator] ()
    {
        const std::string_view partition {currentStart, iterator};
        if (partition == ROOT_SELECTOR)
        {
            current = {ROOT_ID};
        }
        else if (partition == ".")
        {
            // Do nothing.
        }
        else if (partition == "..")
        {
            current = FindParent (current);
        }
        else
        {
            current = FindChild (current, partition);
        }
    };

    while (iterator != _path.end ())
    {
        if (*iterator == PATH_SEPARATOR)
        {
            if (currentStart != iterator)
            {
                processPathStep ();
                if (current.type == ObjectType::INVALID)
                {
                    return current;
                }
            }

            currentStart = iterator + 1u;
        }

        ++iterator;
    }

    if (currentStart != iterator)
    {
        processPathStep ();
        if (current.type == ObjectType::INVALID)
        {
            return current;
        }
    }

    return current;
}

Object VirtualFileSystem::FindParent (const Object &_object) const noexcept
{
    switch (_object.type)
    {
    case ObjectType::INVALID:
        EMERGENCE_ASSERT (false);
        return {};

    case ObjectType::ENTRY:
    {
        auto entryCursor = entriesById.ReadPoint (&_object.entryId);
        if (const auto *entry = static_cast<const Entry *> (*entryCursor))
        {
            if (entry->parentId == INVALID_ID)
            {
                EMERGENCE_ASSERT (entry->id == ROOT_ID);
                EMERGENCE_LOG (ERROR, "VirtualFileSystem: Encountered attempt to get parent of the root.");
                return {};
            }

            return {entry->parentId};
        }

        EMERGENCE_ASSERT (false);
        return {};
    }

    case ObjectType::PATH:
    {
        const std::filesystem::path path {_object.path};
        const std::filesystem::path parentPath = path.parent_path ();

        // Check if current path is mount point and jump back to virtual file system if it is.
        for (auto cursor = fileSystemLinkEntries.ReadSignaled ();
             const auto *entry = static_cast<const Entry *> (*cursor); ++cursor)
        {
            if (entry->filesystemLink == parentPath)
            {
                return {entry->id};
            }
        }

        return {parentPath.generic_string<char, std::char_traits<char>, Memory::HeapSTD<char>> ()};
    }
    }

    EMERGENCE_ASSERT (false);
    return {};
}

Object VirtualFileSystem::FindChild (const Object &_object, const std::string_view &_name) const noexcept
{
    switch (_object.type)
    {
    case ObjectType::INVALID:
        EMERGENCE_ASSERT (false);
        break;

    case ObjectType::ENTRY:
    {
        auto entryCursor = entriesById.ReadPoint (&_object.entryId);
        if (const auto *entry = static_cast<const Entry *> (*entryCursor))
        {
            switch (entry->type)
            {
            case EntryType::VIRTUAL_DIRECTORY:
            {
                struct
                {
                    EntryId parentId;
                    Memory::UniqueString childName;
                } query {_object.entryId, Memory::UniqueString {_name}};

                auto childEntryCursor = entriesByParentIdAndName.ReadPoint (&query);
                if (const auto *childEntry = static_cast<const Entry *> (*childEntryCursor))
                {
                    return {childEntry->id};
                }

                EMERGENCE_LOG (WARNING, "VirtualFileSystem: Unable to find child \"", _name, "\" of path \"",
                               ExtractFullVirtualPath (_object), "\".");
                return {};
            }

            case EntryType::PACKAGE_FILE:
                EMERGENCE_LOG (WARNING, "VirtualFileSystem: Unable to get child \"", _name, "\" because path \"",
                               ExtractFullVirtualPath (_object),
                               "\" points to package file that cannot have children.");
                return {};

            case EntryType::FILE_SYSTEM_LINK:
            {
                const std::filesystem::path childPath = std::filesystem::path (entry->filesystemLink).append (_name);
                if (!std::filesystem::exists (childPath))
                {
                    EMERGENCE_LOG (WARNING, "VirtualFileSystem: Unable to find child \"", _name, "\" of path \"",
                                   ExtractFullVirtualPath (_object), "\".");
                    return {};
                }

                return {childPath.generic_string<char, std::char_traits<char>, Memory::HeapSTD<char>> ()};
            }

            case EntryType::WEAK_FILE_LINK:
                EMERGENCE_LOG (WARNING, "VirtualFileSystem: Unable to get child \"", _name, "\" because path \"",
                               ExtractFullVirtualPath (_object),
                               "\" points to weak file link that cannot have children.");
                return {};
            }
        }
        else
        {
            EMERGENCE_ASSERT (false);
            return {};
        }

        break;
    }

    case ObjectType::PATH:
    {
        const std::filesystem::path childPath = std::filesystem::path (_object.path).append (_name);
        if (!std::filesystem::exists (childPath))
        {
            EMERGENCE_LOG (WARNING, "VirtualFileSystem: Unable to find child \"", _name, "\" of path \"",
                           ExtractFullVirtualPath (_object), "\".");
            return {};
        }

        return {childPath.generic_string<char, std::char_traits<char>, Memory::HeapSTD<char>> ()};
    }
    }

    EMERGENCE_ASSERT (false);
    return {};
}

Container::Utf8String VirtualFileSystem::ExtractFullVirtualPath (const Object &_object) const noexcept
{
    switch (_object.type)
    {
    case ObjectType::INVALID:
        EMERGENCE_ASSERT (false);
        return {};

    case ObjectType::ENTRY:
    {
        auto entryCursor = entriesById.ReadPoint (&_object.entryId);
        if (const auto *entry = static_cast<const Entry *> (*entryCursor))
        {
            if (entry->parentId != INVALID_ID)
            {
                return ExtractFullVirtualPath ({entry->parentId}) + PATH_SEPARATOR + *entry->name;
            }

            return *entry->name;
        }

        EMERGENCE_ASSERT (false);
        return {};
    }

    case ObjectType::PATH:
        return ExtractFullVirtualPath (FindParent (_object)) + PATH_SEPARATOR +
               std::filesystem::path (_object.path)
                   .filename ()
                   .generic_string<char, std::char_traits<char>, Memory::HeapSTD<char>> ();
    }

    EMERGENCE_ASSERT (false);
    return {};
}

Object VirtualFileSystem::CreateFile (const Object &_parent, const std::string_view &_fileName) noexcept
{
    auto createFileAtPath = [] (const std::filesystem::path &_path) -> Object
    {
        if (std::filesystem::exists (_path))
        {
            EMERGENCE_LOG (ERROR, "VirtualFileSystem: Failed to create \"",
                           _path.generic_string<char, std::char_traits<char>, Memory::HeapSTD<char>> (),
                           "\" as it already exists.");
            return {};
        }

        std::ofstream output {_path, std::ios::binary};
        if (output)
        {
            return {_path.generic_string<char, std::char_traits<char>, Memory::HeapSTD<char>> ()};
        }

        EMERGENCE_LOG (ERROR, "VirtualFileSystem: Failed to create \"",
                       _path.generic_string<char, std::char_traits<char>, Memory::HeapSTD<char>> (),
                       "\", cannot open it for write.");
        return {};
    };

    switch (_parent.type)
    {
    case ObjectType::INVALID:
        EMERGENCE_LOG (ERROR, "VirtualFileSystem: Received incorrect parent for file creation.");
        return {};

    case ObjectType::ENTRY:
    {
        auto entryCursor = entriesById.ReadPoint (&_parent.entryId);
        if (const auto *entry = static_cast<const Entry *> (*entryCursor))
        {
            if (entry->type == EntryType::FILE_SYSTEM_LINK)
            {
                return createFileAtPath (std::filesystem::path (entry->filesystemLink).append (_fileName));
            }

            EMERGENCE_LOG (ERROR, "VirtualFileSystem: Unable to create child file for \"",
                           ExtractFullVirtualPath (_parent),
                           "\" as it does not point to any real file system location.");
            return {};
        }

        EMERGENCE_ASSERT (false);
        return {};
    }

    case ObjectType::PATH:
        return createFileAtPath (std::filesystem::path (_parent.path).append (_fileName));
    }

    EMERGENCE_ASSERT (false);
    return {};
}

Object VirtualFileSystem::CreateDirectory (const Object &_parent, const std::string_view &_directoryName) noexcept
{
    switch (_parent.type)
    {
    case ObjectType::INVALID:
        EMERGENCE_LOG (ERROR, "VirtualFileSystem: Received incorrect parent for file creation.");
        return {};

    case ObjectType::ENTRY:
    {
        bool createVirtualDirectory = false;
        {
            auto entryCursor = entriesById.ReadPoint (&_parent.entryId);
            if (const auto *entry = static_cast<const Entry *> (*entryCursor))
            {
                switch (entry->type)
                {
                case EntryType::VIRTUAL_DIRECTORY:
                    createVirtualDirectory = true;
                    break;

                case EntryType::PACKAGE_FILE:
                    EMERGENCE_LOG (ERROR, "VirtualFileSystem: Unable to create child directory \"", _directoryName,
                                   "\" because path \"", ExtractFullVirtualPath (_parent),
                                   "\" points to package file that cannot have children.");
                    return {};

                case EntryType::FILE_SYSTEM_LINK:
                {
                    const std::filesystem::path resultPath =
                        std::filesystem::path (entry->filesystemLink).append (_directoryName);

                    if (!std::filesystem::create_directory (resultPath))
                    {
                        EMERGENCE_LOG (ERROR, "VirtualFileSystem: Unable to create child directory \"", _directoryName,
                                       "\" for \"", ExtractFullVirtualPath (_parent), "\".");
                    }

                    return {resultPath.generic_string<char, std::char_traits<char>, Memory::HeapSTD<char>> ()};
                }

                case EntryType::WEAK_FILE_LINK:
                    EMERGENCE_LOG (ERROR, "VirtualFileSystem: Unable to create child directory \"", _directoryName,
                                   "\" because path \"", ExtractFullVirtualPath (_parent),
                                   "\" points to weak file link that cannot have children.");
                    return {};
                }
            }
        }

        if (createVirtualDirectory)
        {
            auto inserter = entries.AllocateAndInsert ();
            auto *entry = static_cast<Entry *> (inserter.Allocate ());
            entry->id = nextEntryId++;
            entry->parentId = _parent.entryId;
            entry->name = Memory::UniqueString {_directoryName};
            entry->type = EntryType::VIRTUAL_DIRECTORY;
            return {entry->id};
        }

        EMERGENCE_ASSERT (false);
        return {};
    }

    case ObjectType::PATH:
    {
        const std::filesystem::path resultPath = std::filesystem::path (_parent.path).append (_directoryName);
        if (!std::filesystem::create_directory (resultPath))
        {
            EMERGENCE_LOG (ERROR, "VirtualFileSystem: Unable to create child directory \"", _directoryName, "\" for \"",
                           ExtractFullVirtualPath (_parent), "\".");
        }

        return {resultPath.generic_string<char, std::char_traits<char>, Memory::HeapSTD<char>> ()};
    }
    }

    EMERGENCE_ASSERT (false);
    return {};
}

Object VirtualFileSystem::CreateWeakFileLink (const Object &_target,
                                              const Object &_parent,
                                              const std::string_view &_linkName) noexcept
{
    if (_target.type == ObjectType::INVALID)
    {
        EMERGENCE_LOG (ERROR, "VirtualFileSystem: Received invalid target for weak file link creation.");
        return {};
    }

    switch (_parent.type)
    {
    case ObjectType::INVALID:
        EMERGENCE_LOG (ERROR, "VirtualFileSystem: Received invalid parent for weak file link creation.");
        return {};

    case ObjectType::ENTRY:
    {
        {
            auto entryCursor = entriesById.ReadPoint (&_parent.entryId);
            const auto *entry = static_cast<const Entry *> (*entryCursor);
            EMERGENCE_ASSERT (entry);

            if (entry->type != EntryType::VIRTUAL_DIRECTORY)
            {
                EMERGENCE_LOG (ERROR,
                               "VirtualFileSystem: Weak file links are only supported as virtual directory children.");
                return {};
            }
        }

        auto inserter = entries.AllocateAndInsert ();
        auto *entry = static_cast<Entry *> (inserter.Allocate ());
        entry->id = nextEntryId++;
        entry->parentId = _parent.entryId;
        entry->name = Memory::UniqueString {_linkName};
        entry->type = EntryType::WEAK_FILE_LINK;
        new (&entry->weakFileLink) Object {_target};
        return {entry->id};
    }

    case ObjectType::PATH:
        EMERGENCE_LOG (ERROR, "VirtualFileSystem: Weak file links are only supported as virtual directory children.");
        return {};
    }

    EMERGENCE_ASSERT (false);
    return {};
}

Object VirtualFileSystem::MakeDirectories (const Object &_parent, const std::string_view &_relativePath) noexcept
{
    if (_parent.type == ObjectType::INVALID)
    {
        EMERGENCE_ASSERT (false);
        return {};
    }

    Object current = _parent;
    auto currentStart = _relativePath.begin ();
    auto iterator = _relativePath.begin ();

    auto processPathStep = [this, &current, &currentStart, &iterator] ()
    {
        const std::string_view partition {currentStart, iterator};
        if (partition == ROOT_SELECTOR)
        {
            current = {ROOT_ID};
        }
        else if (partition == ".")
        {
            // Do nothing.
        }
        else if (partition == "..")
        {
            current = FindParent (current);
        }
        else
        {
            Object previous = current;
            current = FindChild (current, partition);

            switch (current.type)
            {
            case ObjectType::INVALID:
                current = CreateDirectory (previous, partition);
                break;

            case ObjectType::ENTRY:
            {
                auto entryCursor = entriesById.ReadPoint (&current.entryId);
                const auto *entry = static_cast<const Entry *> (*entryCursor);
                EMERGENCE_ASSERT (entry);

                switch (entry->type)
                {
                case EntryType::VIRTUAL_DIRECTORY:
                case EntryType::FILE_SYSTEM_LINK:
                    break;

                case EntryType::PACKAGE_FILE:
                case EntryType::WEAK_FILE_LINK:
                    EMERGENCE_LOG (ERROR, "VirtualFileSystem: Unable to serve make directories request as \"",
                                   ExtractFullVirtualPath (current), "\" is file!");
                    current = {};
                }

                break;
            }

            case ObjectType::PATH:
                if (!std::filesystem::is_directory (current.path))
                {
                    EMERGENCE_LOG (ERROR, "VirtualFileSystem: Unable to serve make directories request as \"",
                                   current.path, "\" is not a directory!");
                    current = {};
                }

                break;
            }
        }
    };

    while (iterator != _relativePath.end ())
    {
        if (*iterator == PATH_SEPARATOR)
        {
            if (currentStart != iterator)
            {
                processPathStep ();
                if (current.type == ObjectType::INVALID)
                {
                    return current;
                }
            }

            currentStart = iterator + 1u;
        }

        ++iterator;
    }

    if (currentStart != iterator)
    {
        processPathStep ();
    }

    return current;
}

bool VirtualFileSystem::Delete (const Object &_entry, bool _recursive, bool _includingFileSystem) noexcept
{
    switch (_entry.type)
    {
    case ObjectType::INVALID:
        return false;

    case ObjectType::ENTRY:
    {
        Container::Vector<EntryId> children {Memory::Profiler::AllocationGroup {
            Memory::Profiler::AllocationGroup {"VirtualFileSystem"_us}, "AlgorithmTemporary"_us}};

        for (auto cursor = entriesByParentId.ReadPoint (&_entry.entryId);
             const auto *entry = static_cast<const Entry *> (*cursor); ++cursor)
        {
            if (!_recursive)
            {
                // Found children: cannot delete unless recursive deletion is requested.
                return false;
            }

            children.push_back (entry->id);
        }

        bool deletedSuccessfully = true;
        for (EntryId childId : children)
        {
            deletedSuccessfully &= Delete (childId, _recursive, _includingFileSystem);
        }

        auto cursor = entriesById.EditPoint (&_entry.entryId);
        auto *entry = static_cast<Entry *> (*cursor);
        EMERGENCE_ASSERT (entry);

        if (entry)
        {
            if (_includingFileSystem && entry->type == EntryType::FILE_SYSTEM_LINK && _recursive)
            {
                deletedSuccessfully &= std::filesystem::remove_all (entry->filesystemLink) > 0u;
            }

            ~cursor;
        }

        return deletedSuccessfully;
    }

    case ObjectType::PATH:
        if (_includingFileSystem)
        {
            if (_recursive)
            {
                return std::filesystem::remove_all (_entry.path) > 0;
            }

            return std::filesystem::remove (_entry.path);
        }

        return true;
    }

    EMERGENCE_ASSERT (false);
    return false;
}

bool VirtualFileSystem::Mount (const Object &_at, const MountConfiguration &_configuration) noexcept
{
    if (_at.type != ObjectType::ENTRY)
    {
        EMERGENCE_LOG (ERROR, "VirtualFileSystem: Unable to mount at \"", ExtractFullVirtualPath (_at),
                       "\" as it is not a VFS entry.");
        return false;
    }

    if (!std::filesystem::exists (_configuration.sourcePath))
    {
        EMERGENCE_LOG (ERROR, "VirtualFileSystem: Unable to mount \"", _configuration.sourcePath,
                       "\" as it does not exists!");
        return false;
    }

    {
        auto entryCursor = entriesById.ReadPoint (&_at.entryId);
        const auto *entry = static_cast<const Entry *> (*entryCursor);

        if (!entry || entry->type != EntryType::VIRTUAL_DIRECTORY)
        {
            EMERGENCE_LOG (ERROR, "VirtualFileSystem: Unable to mount at \"", ExtractFullVirtualPath (_at),
                           "\" as it is not a virtual directory.");
            return false;
        }
    }

    EMERGENCE_ASSERT (!_configuration.targetPath.ends_with (PATH_SEPARATOR));
    Object nearestParent = _at;
    const std::size_t lastSeparatorPosition = _configuration.targetPath.find_last_of (PATH_SEPARATOR);

    if (lastSeparatorPosition != std::string::npos)
    {
        nearestParent =
            MakeDirectories (_at, std::string_view {_configuration.targetPath.c_str (), lastSeparatorPosition});

        if (nearestParent.type != ObjectType::ENTRY)
        {
            EMERGENCE_LOG (ERROR, "VirtualFileSystem: Unable to mount \"", _configuration.sourcePath,
                           "\" as its target path \"", _configuration.targetPath, "\" does not maps to VFS.");
            return false;
        }
    }

    switch (_configuration.source)
    {
    case MountSource::FILE_SYSTEM:
    {
        if (!std::filesystem::is_directory (_configuration.sourcePath))
        {
            EMERGENCE_LOG (ERROR, "VirtualFileSystem: Unable to mount \"", _configuration.sourcePath,
                           "\" as it is not a directory!");
            return false;
        }

        auto inserter = entries.AllocateAndInsert ();
        auto *mountedEntry = static_cast<Entry *> (inserter.Allocate ());
        mountedEntry->id = nextEntryId++;
        mountedEntry->parentId = nearestParent.entryId;

        mountedEntry->name = Memory::UniqueString {lastSeparatorPosition == std::string::npos ?
                                                       _configuration.targetPath.c_str () :
                                                       &_configuration.targetPath[lastSeparatorPosition + 1u]};

        mountedEntry->type = EntryType::FILE_SYSTEM_LINK;
        new (&mountedEntry->filesystemLink) Container::Utf8String {_configuration.sourcePath};
        return true;
    }

    case MountSource::PACKAGE:
    {
        if (!std::filesystem::is_regular_file (_configuration.sourcePath))
        {
            EMERGENCE_LOG (ERROR, "VirtualFileSystem: Unable to mount \"", _configuration.sourcePath,
                           "\" as it is not a package file!");
            return false;
        }

        std::ifstream input {_configuration.sourcePath.c_str (), std::ios::binary};
        if (!input)
        {
            EMERGENCE_LOG (ERROR, "VirtualFileSystem: Unable to mount \"", _configuration.sourcePath,
                           "\", failed to open package file!");
            return false;
        }

        PackageHeader header;
        if (!Serialization::Binary::DeserializeObject (input, &header, PackageHeader::Reflect ().mapping, {}))
        {
            EMERGENCE_LOG (ERROR, "VirtualFileSystem: Unable to mount \"", _configuration.sourcePath,
                           "\", failed to deserialize package header!");
            return false;
        }

        const std::uint64_t headerSize = static_cast<std::uint64_t> (input.tellg ());
        Object packageRootObject;

        {
            auto inserter = entries.AllocateAndInsert ();
            auto *packageRoot = static_cast<Entry *> (inserter.Allocate ());
            packageRoot->id = nextEntryId++;
            packageRoot->parentId = nearestParent.entryId;

            packageRoot->name = Memory::UniqueString {lastSeparatorPosition == std::string::npos ?
                                                          _configuration.targetPath.c_str () :
                                                          &_configuration.targetPath[lastSeparatorPosition + 1u]};

            packageRoot->type = EntryType::VIRTUAL_DIRECTORY;
            packageRootObject = {packageRoot->id};
        }

        bool anyErrors = false;
        for (const PackageHeaderEntry &headerEntry : header.entries)
        {
            Object entryNearestParent = packageRootObject;
            EMERGENCE_ASSERT (!headerEntry.relativePath.ends_with (PATH_SEPARATOR));
            std::size_t entryLastSeparatorPosition = headerEntry.relativePath.find_last_of (PATH_SEPARATOR);

            if (entryLastSeparatorPosition != std::string::npos)
            {
                entryNearestParent =
                    MakeDirectories (packageRootObject,
                                     std::string_view {headerEntry.relativePath.c_str (), entryLastSeparatorPosition});

                if (entryNearestParent.type != ObjectType::ENTRY)
                {
                    EMERGENCE_LOG (ERROR, "VirtualFileSystem: Unable to add package file \"", headerEntry.relativePath,
                                   "\" as its target path does not maps to VFS.");
                    anyErrors = true;
                    continue;
                }
            }

            auto inserter = entries.AllocateAndInsert ();
            auto *entry = static_cast<Entry *> (inserter.Allocate ());
            entry->id = nextEntryId++;
            entry->parentId = entryNearestParent.entryId;

            entry->name = Memory::UniqueString {entryLastSeparatorPosition == std::string::npos ?
                                                    headerEntry.relativePath.c_str () :
                                                    &headerEntry.relativePath[entryLastSeparatorPosition + 1u]};

            entry->type = EntryType::PACKAGE_FILE;
            new (&entry->packageFile) PackageFileData {};
            entry->packageFile.path = _configuration.sourcePath;
            entry->packageFile.offset = headerSize + headerEntry.offset;
            entry->packageFile.size = headerEntry.size;
        }

        return !anyErrors;
    }
    }

    EMERGENCE_ASSERT (false);
    return false;
}

EntryType VirtualFileSystem::GetEntryType (EntryId _id) const noexcept
{
    auto entryCursor = entriesById.ReadPoint (&_id);
    const auto *entry = static_cast<const Entry *> (*entryCursor);
    EMERGENCE_ASSERT (entry);
    return entry->type;
}

Memory::UniqueString VirtualFileSystem::GetEntryName (EntryId _id) const noexcept
{
    auto entryCursor = entriesById.ReadPoint (&_id);
    const auto *entry = static_cast<const Entry *> (*entryCursor);
    EMERGENCE_ASSERT (entry);
    return entry->name;
}

Container::Utf8String VirtualFileSystem::GetPackageFilePath (EntryId _id) const noexcept
{
    auto entryCursor = entriesById.ReadPoint (&_id);
    const auto *entry = static_cast<const Entry *> (*entryCursor);
    EMERGENCE_ASSERT (entry);
    EMERGENCE_ASSERT (entry->type == EntryType::PACKAGE_FILE);
    return entry->packageFile.path;
}

Object VirtualFileSystem::GetWeakFileLinkTarget (EntryId _id) const noexcept
{
    auto entryCursor = entriesById.ReadPoint (&_id);
    const auto *entry = static_cast<const Entry *> (*entryCursor);
    EMERGENCE_ASSERT (entry);
    EMERGENCE_ASSERT (entry->type == EntryType::WEAK_FILE_LINK);
    return entry->weakFileLink;
}

FileReadContext VirtualFileSystem::OpenFileForRead (const Object &_object) const noexcept
{
    switch (_object.type)
    {
    case ObjectType::INVALID:
        return {};

    case ObjectType::ENTRY:
    {
        auto entryCursor = entriesById.ReadPoint (&_object.entryId);
        const auto *entry = static_cast<const Entry *> (*entryCursor);
        EMERGENCE_ASSERT (entry);

        switch (entry->type)
        {
        case EntryType::VIRTUAL_DIRECTORY:
        case EntryType::FILE_SYSTEM_LINK:
            EMERGENCE_LOG (ERROR, "VirtualFileSystem: Unable to open file \"", ExtractFullVirtualPath (_object),
                           "\" for read: it points to directory instead of file.");
            return {};

        case EntryType::PACKAGE_FILE:
        {
            FILE *packageFile = fopen (entry->packageFile.path.c_str (), "rb");
            fseek (packageFile, static_cast<long> (entry->packageFile.offset), SEEK_SET);
            return {packageFile, entry->packageFile.offset, entry->packageFile.size};
        }

        case EntryType::WEAK_FILE_LINK:
            return OpenFileForRead (entry->weakFileLink);
        }

        EMERGENCE_ASSERT (false);
        return {};
    }

    case ObjectType::PATH:
    {
        FILE *file = fopen (_object.path.c_str (), "rb");
        std::uint64_t size = 0u;

        if (file)
        {
            fseek (file, 0u, SEEK_END);
            size = static_cast<std::uint64_t> (ftell (file));
            fseek (file, 0u, SEEK_SET);
        }

        return {file, 0u, size};
    }
    }

    EMERGENCE_ASSERT (false);
    return {};
}

FileWriteContext VirtualFileSystem::OpenFileForWrite (const Object &_object) const noexcept
{
    switch (_object.type)
    {
    case ObjectType::INVALID:
        return {};

    case ObjectType::ENTRY:
    {
        auto entryCursor = entriesById.ReadPoint (&_object.entryId);
        const auto *entry = static_cast<const Entry *> (*entryCursor);
        EMERGENCE_ASSERT (entry);

        switch (entry->type)
        {
        case EntryType::VIRTUAL_DIRECTORY:
        case EntryType::FILE_SYSTEM_LINK:
            EMERGENCE_LOG (ERROR, "VirtualFileSystem: Unable to open file \"", ExtractFullVirtualPath (_object),
                           "\" for write: it points to directory instead of file.");
            return {};

        case EntryType::PACKAGE_FILE:
            EMERGENCE_LOG (ERROR, "VirtualFileSystem: Unable to open file \"", ExtractFullVirtualPath (_object),
                           "\" for write: package files are read-only.");
            return {};

        case EntryType::WEAK_FILE_LINK:
            return OpenFileForWrite (entry->weakFileLink);
        }

        EMERGENCE_ASSERT (false);
        return {};
    }

    case ObjectType::PATH:
    {
        return {fopen (_object.path.c_str (), "wb")};
    }
    }

    EMERGENCE_ASSERT (false);
    return {};
}
} // namespace Emergence::VirtualFileSystem::Original

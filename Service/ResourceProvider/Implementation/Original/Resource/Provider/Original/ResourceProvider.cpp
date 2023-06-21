#define _CRT_SECURE_NO_WARNINGS

#include <Log/Log.hpp>

#include <Resource/Provider/IndexFile.hpp>
#include <Resource/Provider/Original/ResourceProvider.hpp>

#include <Serialization/Binary.hpp>
#include <Serialization/Yaml.hpp>

#include <StandardLayout/MappingRegistration.hpp>

#include <VirtualFileSystem/Reader.hpp>
#include <VirtualFileSystem/Writer.hpp>

namespace Emergence::Resource::Provider::Original
{
enum class ObjectResourceFormat : std::uint8_t
{
    BINARY = 0u,
    YAML
};

struct ObjectResourceData final
{
    Memory::UniqueString id;
    StandardLayout::Mapping type;
    Memory::UniqueString source;
    VirtualFileSystem::Entry entry;
    ObjectResourceFormat format = ObjectResourceFormat::BINARY;

    struct Reflection final
    {
        StandardLayout::FieldId id;
        StandardLayout::FieldId typeNumber;
        StandardLayout::FieldId source;
        StandardLayout::FieldId relativePath;
        StandardLayout::FieldId format;
        StandardLayout::Mapping mapping;
    };

    static const Reflection &Reflect () noexcept;
};

const ObjectResourceData::Reflection &ObjectResourceData::Reflect () noexcept
{
    static const Reflection reflection = [] ()
    {
        EMERGENCE_MAPPING_REGISTRATION_BEGIN (ObjectResourceData);
        EMERGENCE_MAPPING_REGISTER_REGULAR (id);

        static_assert (sizeof (StandardLayout::Mapping) == sizeof (std::uint64_t) ||
                       sizeof (StandardLayout::Mapping) == sizeof (std::uint32_t));

        if constexpr (sizeof (StandardLayout::Mapping) == sizeof (std::uint64_t))
        {
            reflectionData.typeNumber =
                builder.RegisterUInt64 (Memory::UniqueString {"typeBlock"}, offsetof (ObjectResourceData, type));
        }
        else if constexpr (sizeof (StandardLayout::Mapping) == sizeof (std::uint32_t))
        {
            reflectionData.typeNumber =
                builder.RegisterUInt32 (Memory::UniqueString {"typeBlock"}, offsetof (ObjectResourceData, type));
        }

        EMERGENCE_MAPPING_REGISTER_REGULAR (source);
        EMERGENCE_MAPPING_REGISTER_REGULAR (format);
        EMERGENCE_MAPPING_REGISTRATION_END ();
    }();

    return reflection;
}

struct ThirdPartyResourceData final
{
    Memory::UniqueString id;
    Memory::UniqueString source;
    VirtualFileSystem::Entry entry;

    struct Reflection final
    {
        StandardLayout::FieldId id;
        StandardLayout::FieldId source;
        StandardLayout::FieldId relativePath;
        StandardLayout::Mapping mapping;
    };

    static const Reflection &Reflect () noexcept;
};

const ThirdPartyResourceData::Reflection &ThirdPartyResourceData::Reflect () noexcept
{
    static const Reflection reflection = [] ()
    {
        EMERGENCE_MAPPING_REGISTRATION_BEGIN (ThirdPartyResourceData);
        EMERGENCE_MAPPING_REGISTER_REGULAR (id);
        EMERGENCE_MAPPING_REGISTER_REGULAR (source);
        EMERGENCE_MAPPING_REGISTRATION_END ();
    }();

    return reflection;
}

ResourceProvider::ObjectRegistryCursor::ObjectRegistryCursor (
    const ResourceProvider::ObjectRegistryCursor &_other) noexcept
    : owner (_other.owner),
      cursor (_other.cursor)
{
    EMERGENCE_ASSERT (owner);
}

ResourceProvider::ObjectRegistryCursor::ObjectRegistryCursor (ResourceProvider::ObjectRegistryCursor &&_other) noexcept
    : owner (_other.owner),
      cursor (std::move (_other.cursor))
{
    _other.owner = nullptr;
}

Memory::UniqueString ResourceProvider::ObjectRegistryCursor::operator* () const noexcept
{
    if (const auto *object = static_cast<const ObjectResourceData *> (*cursor))
    {
        return object->id;
    }

    return {};
}

ResourceProvider::ObjectRegistryCursor &ResourceProvider::ObjectRegistryCursor::operator++ () noexcept
{
    ++cursor;
    return *this;
}

ResourceProvider::ObjectRegistryCursor::ObjectRegistryCursor (
    const ResourceProvider *_owner, RecordCollection::PointRepresentation::ReadCursor _cursor) noexcept
    : owner (_owner),
      cursor (std::move (_cursor))
{
    EMERGENCE_ASSERT (owner);
}

ResourceProvider::ResourceProvider (VirtualFileSystem::Context *_virtualFileSystemContext,
                                    Container::MappingRegistry _objectTypesRegistry,
                                    Container::MappingRegistry _patchableTypesRegistry) noexcept
    : virtualFileSystemContext (_virtualFileSystemContext),

      objects (ObjectResourceData::Reflect ().mapping),
      objectsById (objects.CreatePointRepresentation ({ObjectResourceData::Reflect ().id})),
      objectsByType (objects.CreatePointRepresentation ({ObjectResourceData::Reflect ().typeNumber})),
      objectsBySource (objects.CreatePointRepresentation ({ObjectResourceData::Reflect ().source})),

      thirdPartyResources (ThirdPartyResourceData::Reflect ().mapping),
      thirdPartyResourcesById (thirdPartyResources.CreatePointRepresentation ({ThirdPartyResourceData::Reflect ().id})),
      thirdPartyResourcesBySource (
          thirdPartyResources.CreatePointRepresentation ({ThirdPartyResourceData::Reflect ().source})),

      objectTypesRegistry (std::move (_objectTypesRegistry)),
      patchableTypesRegistry (std::move (_patchableTypesRegistry))
{
}

const Container::MappingRegistry &ResourceProvider::GetObjectTypesRegistry () const noexcept
{
    return objectTypesRegistry;
}

const Container::MappingRegistry &ResourceProvider::GetPatchableTypesRegistry () const noexcept
{
    return patchableTypesRegistry;
}

SourceOperationResponse ResourceProvider::AddSource ([[maybe_unused]] Memory::UniqueString _path) noexcept
{
    if (auto cursor = objectsBySource.ReadPoint (&_path); *cursor)
    {
        return SourceOperationResponse::ALREADY_EXIST;
    }

    if (auto cursor = thirdPartyResourcesBySource.ReadPoint (&_path); *cursor)
    {
        return SourceOperationResponse::ALREADY_EXIST;
    }

    const VirtualFileSystem::Entry indexFileEntry {*virtualFileSystemContext,
                                                   EMERGENCE_BUILD_STRING (_path, "/", IndexFile::INDEX_FILE_NAME)};

    if (indexFileEntry.GetType () == VirtualFileSystem::EntryType::FILE)
    {
        return AddSourceFromIndex (indexFileEntry, _path);
    }

    EMERGENCE_LOG (WARNING, "ResourceProvider: No index for source \"", _path, "\", scanning file system!");
    return AddSourceThroughScan (_path);
}

SourceOperationResponse ResourceProvider::SaveSourceIndex (
    [[maybe_unused]] Memory::UniqueString _sourcePath) const noexcept
{
    const VirtualFileSystem::Entry sourceEntry {*virtualFileSystemContext, *_sourcePath};
    if (sourceEntry.GetType () != VirtualFileSystem::EntryType::DIRECTORY)
    {
        return SourceOperationResponse::NOT_FOUND;
    }

    const Container::Utf8String sourceFullPath = sourceEntry.GetFullPath ();
    IndexFile content;

    for (auto cursor = objectsBySource.ReadPoint (&_sourcePath);
         const auto *objectData = static_cast<const ObjectResourceData *> (*cursor); ++cursor)
    {
        IndexFileObjectItem &objectItem = content.objects.emplace_back ();
        objectItem.id = objectData->id;
        objectItem.typeName = objectData->type.GetName ();

        const Container::Utf8String objectFullPath = objectData->entry.GetFullPath ();
        EMERGENCE_ASSERT (objectFullPath.starts_with (sourceFullPath));
        EMERGENCE_ASSERT (objectFullPath[sourceFullPath.size ()] == VirtualFileSystem::PATH_SEPARATOR);
        EMERGENCE_ASSERT (objectFullPath.size () > sourceFullPath.size () + 1u);
        objectItem.relativePath = objectFullPath.substr (sourceFullPath.size () + 1u);
    }

    for (auto cursor = thirdPartyResourcesBySource.ReadPoint (&_sourcePath);
         const auto *resourceData = static_cast<const ThirdPartyResourceData *> (*cursor); ++cursor)
    {
        IndexFileThirdPartyItem &resourceItem = content.thirdParty.emplace_back ();
        resourceItem.id = resourceData->id;

        const Container::Utf8String resourceFullPath = resourceData->entry.GetFullPath ();
        EMERGENCE_ASSERT (resourceFullPath.starts_with (sourceFullPath));
        EMERGENCE_ASSERT (resourceFullPath[sourceFullPath.size ()] == VirtualFileSystem::PATH_SEPARATOR);
        EMERGENCE_ASSERT (resourceFullPath.size () > sourceFullPath.size () + 1u);
        resourceItem.relativePath = resourceFullPath.substr (sourceFullPath.size () + 1u);
    }

    const VirtualFileSystem::Entry indexFile =
        virtualFileSystemContext->CreateFile ({*virtualFileSystemContext, *_sourcePath}, IndexFile::INDEX_FILE_NAME);

    if (!indexFile)
    {
        return SourceOperationResponse::IO_ERROR;
    }

    VirtualFileSystem::Writer writer {indexFile, VirtualFileSystem::OpenMode::BINARY};
    if (!writer)
    {
        return SourceOperationResponse::IO_ERROR;
    }

    Serialization::Binary::SerializeObject (writer.OutputStream (), &content, IndexFile::Reflect ().mapping);
    return SourceOperationResponse::SUCCESSFUL;
}

SourceOperationResponse ResourceProvider::RemoveSource (Memory::UniqueString _path) noexcept
{
    const bool foundAny = ClearSource (_path);
    return foundAny ? SourceOperationResponse::SUCCESSFUL : SourceOperationResponse::NOT_FOUND;
}

LoadingOperationResponse ResourceProvider::LoadObject (const StandardLayout::Mapping &_type,
                                                       Memory::UniqueString _id,
                                                       void *_output) const noexcept
{
    auto cursor = objectsById.ReadPoint (&_id);
    const auto *object = static_cast<const ObjectResourceData *> (*cursor);

    if (!object)
    {
        return LoadingOperationResponse::NOT_FOUND;
    }

    if (object->type != _type)
    {
        return LoadingOperationResponse::WRONG_TYPE;
    }

    if (!object->entry)
    {
        return LoadingOperationResponse::NOT_FOUND;
    }

    // We always open in binary mode as VFS packages do not support text mode.
    VirtualFileSystem::Reader reader {object->entry, VirtualFileSystem::OpenMode::BINARY};

    if (!reader)
    {
        return LoadingOperationResponse::NOT_FOUND;
    }

    switch (object->format)
    {
    case ObjectResourceFormat::BINARY:
    {
        [[maybe_unused]] const Memory::UniqueString typeName =
            Serialization::Binary::DeserializeTypeName (reader.InputStream ());
        EMERGENCE_ASSERT (typeName == _type.GetName ());

        if (!Serialization::Binary::DeserializeObject (reader.InputStream (), _output, _type, patchableTypesRegistry))
        {
            return LoadingOperationResponse::IO_ERROR;
        }

        break;
    }

    case ObjectResourceFormat::YAML:
    {
        // We skip type name deserialization here as it is just a comment.
        if (!Serialization::Yaml::DeserializeObject (reader.InputStream (), _output, _type, patchableTypesRegistry))
        {
            return LoadingOperationResponse::IO_ERROR;
        }

        break;
    }
    }

    return LoadingOperationResponse::SUCCESSFUL;
}

LoadingOperationResponse ResourceProvider::LoadThirdPartyResource (Memory::UniqueString _id,
                                                                   Memory::Heap &_allocator,
                                                                   std::uint64_t &_sizeOutput,
                                                                   std::uint8_t *&_dataOutput) const noexcept
{
    auto cursor = thirdPartyResourcesById.ReadPoint (&_id);
    const auto *resource = static_cast<const ThirdPartyResourceData *> (*cursor);

    if (!resource)
    {
        return LoadingOperationResponse::NOT_FOUND;
    }

    if (!resource->entry)
    {
        return LoadingOperationResponse::NOT_FOUND;
    }

    VirtualFileSystem::Reader reader {resource->entry, VirtualFileSystem::OpenMode::BINARY};
    if (!reader)
    {
        return LoadingOperationResponse::NOT_FOUND;
    }

    reader.InputStream ().seekg (0u, std::ios::end);
    _sizeOutput = static_cast<std::uint64_t> (reader.InputStream ().tellg ());
    reader.InputStream ().seekg (0u, std::ios::beg);
    _dataOutput = static_cast<std::uint8_t *> (_allocator.Acquire (_sizeOutput, alignof (std::uint64_t)));

    if (!reader.InputStream ().read (reinterpret_cast<char *> (_dataOutput),
                                     static_cast<std::streamsize> (_sizeOutput)))
    {
        return LoadingOperationResponse::IO_ERROR;
    }

    return LoadingOperationResponse::SUCCESSFUL;
}

ResourceProvider::ObjectRegistryCursor ResourceProvider::FindObjectsByType (
    const StandardLayout::Mapping &_type) const noexcept
{
    return {this, objectsByType.ReadPoint (&_type)};
}

SourceOperationResponse ResourceProvider::AddSourceFromIndex (const VirtualFileSystem::Entry &_indexFile,
                                                              Memory::UniqueString _path) noexcept
{
    VirtualFileSystem::Reader reader {_indexFile, VirtualFileSystem::OpenMode::BINARY};
    if (!reader)
    {
        return SourceOperationResponse::IO_ERROR;
    }

    IndexFile content;
    if (!Serialization::Binary::DeserializeObject (reader.InputStream (), &content, IndexFile::Reflect ().mapping,
                                                   patchableTypesRegistry))
    {
        return SourceOperationResponse::IO_ERROR;
    }

    for (const IndexFileObjectItem &objectItem : content.objects)
    {
        if (SourceOperationResponse response =
                AddObject (objectItem.id, objectItem.typeName, _path, objectItem.relativePath);
            response != SourceOperationResponse::SUCCESSFUL)
        {
            ClearSource (_path);
            return response;
        }
    }

    for (const IndexFileThirdPartyItem &resourceItem : content.thirdParty)
    {
        if (SourceOperationResponse response =
                AddThirdPartyResource (resourceItem.id, _path, resourceItem.relativePath);
            response != SourceOperationResponse::SUCCESSFUL)
        {
            ClearSource (_path);
            return response;
        }
    }

    return SourceOperationResponse::SUCCESSFUL;
}

SourceOperationResponse ResourceProvider::AddSourceThroughScan (Memory::UniqueString _path) noexcept
{
    const VirtualFileSystem::Entry pathRoot {*virtualFileSystemContext, *_path};
    if (pathRoot.GetType () != VirtualFileSystem::EntryType::DIRECTORY)
    {
        EMERGENCE_LOG (ERROR, "ResourceProvider: Given source directory \"", _path, "\" does not exist.");
        return SourceOperationResponse::NOT_FOUND;
    }

    const Container::Utf8String rootFullPath = pathRoot.GetFullPath ();
    SourceOperationResponse finalResponse = SourceOperationResponse::SUCCESSFUL;
    Container::Vector<VirtualFileSystem::Entry> scanStack {
        Memory::Profiler::AllocationGroup {Memory::UniqueString {"ResourceProviderAlgorithm"}}};
    scanStack.emplace_back (pathRoot);

    while (!scanStack.empty ())
    {
        VirtualFileSystem::Entry scanEntry = scanStack.back ();
        scanStack.pop_back ();

        for (VirtualFileSystem::Entry::Cursor cursor = scanEntry.ReadChildren ();
             VirtualFileSystem::Entry entry = *cursor; ++cursor)
        {
            switch (entry.GetType ())
            {
            case VirtualFileSystem::EntryType::INVALID:
                EMERGENCE_ASSERT (false);
                break;

            case VirtualFileSystem::EntryType::FILE:
            {
                if (entry.GetExtension () == "bin")
                {
                    // Attempt to read type name.
                    VirtualFileSystem::Reader reader {entry, VirtualFileSystem::OpenMode::BINARY};
                    const Memory::UniqueString typeName =
                        Serialization::Binary::DeserializeTypeName (reader.InputStream ());

                    if (!*typeName)
                    {
                        EMERGENCE_LOG (ERROR, "ResourceProvider: Failed to parse object type from \"",
                                       entry.GetFullPath (), "\" of source \"", _path, "\".");
                        finalResponse = SourceOperationResponse::IO_ERROR;
                    }
                    else if (SourceOperationResponse response = AddObject (
                                 Memory::UniqueString {entry.GetFileName ().c_str ()}, typeName, _path, entry);
                             response != SourceOperationResponse::SUCCESSFUL)
                    {
                        finalResponse = response;
                    }
                }
                else if (entry.GetExtension () == "yaml")
                {
                    // Attempt to read type name.
                    // We always open in binary mode as VFS packages do not support text mode.
                    VirtualFileSystem::Reader reader {entry, VirtualFileSystem::OpenMode::BINARY};

                    const Memory::UniqueString typeName =
                        Serialization::Yaml::DeserializeTypeName (reader.InputStream ());

                    if (!*typeName)
                    {
                        EMERGENCE_LOG (ERROR, "ResourceProvider: Failed to parse object type from \"",
                                       entry.GetFullPath (), "\" of source \"", _path, "\".");
                        finalResponse = SourceOperationResponse::IO_ERROR;
                    }
                    else if (SourceOperationResponse response = AddObject (
                                 Memory::UniqueString {entry.GetFileName ().c_str ()}, typeName, _path, entry);
                             response != SourceOperationResponse::SUCCESSFUL)
                    {
                        finalResponse = response;
                    }
                }
                else if (entry.GetFullFileName () != IndexFile::INDEX_FILE_NAME)
                {
                    if (SourceOperationResponse response = AddThirdPartyResource (
                            Memory::UniqueString {entry.GetFullFileName ().c_str ()}, _path, entry);
                        response != SourceOperationResponse::SUCCESSFUL)
                    {
                        finalResponse = response;
                    }
                }

                break;
            }

            case VirtualFileSystem::EntryType::DIRECTORY:
                scanStack.emplace_back (std::move (entry));
                break;
            }
        }
    }

    if (finalResponse != SourceOperationResponse::SUCCESSFUL)
    {
        ClearSource (_path);
    }

    return finalResponse;
}

SourceOperationResponse ResourceProvider::AddObject (Memory::UniqueString _id,
                                                     Memory::UniqueString _typeName,
                                                     Memory::UniqueString _source,
                                                     const Container::Utf8String &_relativePath) noexcept
{
    return AddObject (
        _id, _typeName, _source,
        VirtualFileSystem::Entry {*virtualFileSystemContext,
                                  EMERGENCE_BUILD_STRING (_source, VirtualFileSystem::PATH_SEPARATOR, _relativePath)});
}

SourceOperationResponse ResourceProvider::AddObject (Memory::UniqueString _id,
                                                     Memory::UniqueString _typeName,
                                                     Memory::UniqueString _source,
                                                     const VirtualFileSystem::Entry &_entry) noexcept
{
    StandardLayout::Mapping type = objectTypesRegistry.Get (_typeName);
    if (!type)
    {
        EMERGENCE_LOG (ERROR, "ResourceProvider: Unknown object type \"", _typeName, "\"!");
        return SourceOperationResponse::UNKNOWN_TYPE;
    }

    ObjectResourceFormat format;
    if (_entry.GetExtension () == "bin")
    {
        format = ObjectResourceFormat::BINARY;
    }
    else if (_entry.GetExtension () == "yaml")
    {
        format = ObjectResourceFormat::YAML;
    }
    else
    {
        EMERGENCE_LOG (ERROR, "ResourceProvider: Unknown object format of file \"", _entry.GetFullPath (), "\"!");
        return SourceOperationResponse::UNKNOWN_FORMAT;
    }

    if (auto cursor = objectsById.ReadPoint (&_id); *cursor)
    {
        EMERGENCE_LOG (ERROR, "ResourceProvider: Object id \"", _id, "\" is used several times!");
        return SourceOperationResponse::DUPLICATE_ID;
    }

    auto objectInserter = objects.AllocateAndInsert ();
    auto *objectData = static_cast<ObjectResourceData *> (objectInserter.Allocate ());
    objectData->id = _id;
    objectData->type = type;
    objectData->source = _source;
    objectData->entry = _entry;
    objectData->format = format;
    return SourceOperationResponse::SUCCESSFUL;
}

SourceOperationResponse ResourceProvider::AddThirdPartyResource (Memory::UniqueString _id,
                                                                 Memory::UniqueString _source,
                                                                 const Container::Utf8String &_relativePath) noexcept
{
    return AddThirdPartyResource (
        _id, _source,
        VirtualFileSystem::Entry {*virtualFileSystemContext,
                                  EMERGENCE_BUILD_STRING (_source, VirtualFileSystem::PATH_SEPARATOR, _relativePath)});
}

SourceOperationResponse ResourceProvider::AddThirdPartyResource (Memory::UniqueString _id,
                                                                 Memory::UniqueString _source,
                                                                 const VirtualFileSystem::Entry &_entry) noexcept
{
    if (auto cursor = thirdPartyResourcesById.ReadPoint (&_id); *cursor)
    {
        EMERGENCE_LOG (ERROR, "ResourceProvider: Third party resource id \"", _id, "\" is used several times!");
        return SourceOperationResponse::DUPLICATE_ID;
    }

    auto resourceInserter = thirdPartyResources.AllocateAndInsert ();
    auto *resourceData = static_cast<ThirdPartyResourceData *> (resourceInserter.Allocate ());
    resourceData->id = _id;
    resourceData->source = _source;
    resourceData->entry = _entry;
    return SourceOperationResponse::SUCCESSFUL;
}

bool ResourceProvider::ClearSource (Memory::UniqueString _path) noexcept
{
    bool foundAny = false;
    for (auto cursor = objectsBySource.EditPoint (&_path); *cursor;)
    {
        foundAny = true;
        ~cursor;
    }

    for (auto cursor = thirdPartyResourcesBySource.EditPoint (&_path); *cursor;)
    {
        foundAny = true;
        ~cursor;
    }

    return foundAny;
}
} // namespace Emergence::Resource::Provider::Original

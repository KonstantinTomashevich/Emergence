#define _CRT_SECURE_NO_WARNINGS

#include <filesystem>
#include <fstream>

#include <Log/Log.hpp>

#include <Resource/Provider/IndexFile.hpp>
#include <Resource/Provider/Original/ResourceProvider.hpp>

#include <Serialization/Binary.hpp>
#include <Serialization/Yaml.hpp>

#include <StandardLayout/MappingRegistration.hpp>

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
    Container::Utf8String relativePath;
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
                builder.RegisterUInt64 (Memory::UniqueString {"typeStateBlock"}, offsetof (ObjectResourceData, type));
        }
        else if constexpr (sizeof (StandardLayout::Mapping) == sizeof (std::uint32_t))
        {
            reflectionData.typeNumber =
                builder.RegisterUInt32 (Memory::UniqueString {"typeStateBlock"}, offsetof (ObjectResourceData, type));
        }

        EMERGENCE_MAPPING_REGISTER_REGULAR (source);
        EMERGENCE_MAPPING_REGISTER_REGULAR (relativePath);
        EMERGENCE_MAPPING_REGISTER_REGULAR (format);
        EMERGENCE_MAPPING_REGISTRATION_END ();
    }();

    return reflection;
}

struct ThirdPartyResourceData final
{
    Memory::UniqueString id;
    Memory::UniqueString source;
    Container::Utf8String relativePath;

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
        EMERGENCE_MAPPING_REGISTER_REGULAR (relativePath);
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
    ++owner->dataReadersCount;
}

ResourceProvider::ObjectRegistryCursor::ObjectRegistryCursor (ResourceProvider::ObjectRegistryCursor &&_other) noexcept
    : owner (_other.owner),
      cursor (std::move (_other.cursor))
{
    _other.owner = nullptr;
}

ResourceProvider::ObjectRegistryCursor::~ObjectRegistryCursor () noexcept
{
    if (owner)
    {
        --owner->dataReadersCount;
    }
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
    ++owner->dataReadersCount;
}

ResourceProvider::ResourceProvider (Container::MappingRegistry _objectTypesRegistry,
                                    Container::MappingRegistry _patchableTypesRegistry) noexcept
    : objects (ObjectResourceData::Reflect ().mapping),
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

ResourceProvider::~ResourceProvider () noexcept
{
    EMERGENCE_ASSERT (dataWritersCount == 0u);
    EMERGENCE_ASSERT (dataReadersCount == 0u);
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
    EMERGENCE_ASSERT (dataWritersCount == 0u);
    EMERGENCE_ASSERT (dataReadersCount == 0u);
    ++dataWritersCount;

    if (auto cursor = objectsBySource.ReadPoint (&_path); *cursor)
    {
        --dataWritersCount;
        return SourceOperationResponse::ALREADY_EXIST;
    }

    if (auto cursor = thirdPartyResourcesBySource.ReadPoint (&_path); *cursor)
    {
        --dataWritersCount;
        return SourceOperationResponse::ALREADY_EXIST;
    }

    if (std::filesystem::exists (EMERGENCE_BUILD_STRING (_path, "/", IndexFile::INDEX_FILE_NAME)))
    {
        const SourceOperationResponse response = AddSourceFromIndex (_path);
        --dataWritersCount;
        return response;
    }

    EMERGENCE_LOG (WARNING, "ResourceProvider: No index for source \"", _path, "\", scanning file system!");
    const SourceOperationResponse response = AddSourceThroughScan (_path);
    --dataWritersCount;
    return response;
}

SourceOperationResponse ResourceProvider::SaveSourceIndex (
    [[maybe_unused]] Memory::UniqueString _sourcePath) const noexcept
{
    EMERGENCE_ASSERT (dataWritersCount == 0u);
    ++dataReadersCount;

    IndexFile content;
    for (auto cursor = objectsBySource.ReadPoint (&_sourcePath);
         const auto *objectData = static_cast<const ObjectResourceData *> (*cursor); ++cursor)
    {
        IndexFileObjectItem &objectItem = content.objects.emplace_back ();
        objectItem.id = objectData->id;
        objectItem.typeName = objectData->type.GetName ();
        objectItem.relativePath = objectData->relativePath;
    }

    for (auto cursor = thirdPartyResourcesBySource.ReadPoint (&_sourcePath);
         const auto *resourceData = static_cast<const ThirdPartyResourceData *> (*cursor); ++cursor)
    {
        IndexFileThirdPartyItem &resourceItem = content.thirdParty.emplace_back ();
        resourceItem.id = resourceData->id;
        resourceItem.relativePath = resourceData->relativePath;
    }

    std::ofstream output (EMERGENCE_BUILD_STRING (_sourcePath, "/", IndexFile::INDEX_FILE_NAME), std::ios::binary);
    if (!output)
    {
        --dataReadersCount;
        return SourceOperationResponse::IO_ERROR;
    }

    Serialization::Binary::SerializeObject (output, &content, IndexFile::Reflect ().mapping);
    --dataReadersCount;
    return SourceOperationResponse::SUCCESSFUL;
}

SourceOperationResponse ResourceProvider::RemoveSource (Memory::UniqueString _path) noexcept
{
    EMERGENCE_ASSERT (dataWritersCount == 0u);
    EMERGENCE_ASSERT (dataReadersCount == 0u);

    ++dataWritersCount;
    const bool foundAny = ClearSource (_path);
    --dataWritersCount;
    return foundAny ? SourceOperationResponse::SUCCESSFUL : SourceOperationResponse::NOT_FOUND;
}

LoadingOperationResponse ResourceProvider::LoadObject (const StandardLayout::Mapping &_type,
                                                       Memory::UniqueString _id,
                                                       void *_output) const noexcept
{
    EMERGENCE_ASSERT (dataWritersCount == 0u);
    ++dataReadersCount;

    auto cursor = objectsById.ReadPoint (&_id);
    const auto *object = static_cast<const ObjectResourceData *> (*cursor);

    if (!object)
    {
        --dataReadersCount;
        return LoadingOperationResponse::NOT_FOUND;
    }

    if (object->type != _type)
    {
        --dataReadersCount;
        return LoadingOperationResponse::WRONG_TYPE;
    }

    std::ios_base::openmode openMode = std::ios::in;
    switch (object->format)
    {
    case ObjectResourceFormat::BINARY:
        openMode = std::ios::binary;
        break;

    case ObjectResourceFormat::YAML:
        break;
    }

    std::ifstream input (EMERGENCE_BUILD_STRING (object->source, "/", object->relativePath), openMode);
    if (!input)
    {
        --dataReadersCount;
        return LoadingOperationResponse::NOT_FOUND;
    }

    switch (object->format)
    {
    case ObjectResourceFormat::BINARY:
    {
        EMERGENCE_LOG (ERROR, "Temp log: 6");

        [[maybe_unused]] const Memory::UniqueString typeName = Serialization::Binary::DeserializeTypeName (input);
        EMERGENCE_ASSERT (typeName == _type.GetName ());

        EMERGENCE_LOG (ERROR, "Temp log: 7");

        if (!Serialization::Binary::DeserializeObject (input, _output, _type, patchableTypesRegistry))
        {
            EMERGENCE_LOG (ERROR, "Temp log: ---");

            --dataReadersCount;
            return LoadingOperationResponse::IO_ERROR;
        }

        break;
    }

    case ObjectResourceFormat::YAML:
    {
        EMERGENCE_LOG (ERROR, "Temp log: 8");

        [[maybe_unused]] const Memory::UniqueString typeName = Serialization::Yaml::DeserializeTypeName (input);
        EMERGENCE_ASSERT (typeName == _type.GetName ());

        EMERGENCE_LOG (ERROR, "Temp log: 9");

        if (!Serialization::Yaml::DeserializeObject (input, _output, _type, patchableTypesRegistry))
        {
            EMERGENCE_LOG (ERROR, "Temp log: ---");

            --dataReadersCount;
            return LoadingOperationResponse::IO_ERROR;
        }

        break;
    }
    }

    --dataReadersCount;
    return LoadingOperationResponse::SUCCESSFUL;
}

LoadingOperationResponse ResourceProvider::LoadThirdPartyResource (Memory::UniqueString _id,
                                                                   Memory::Heap &_allocator,
                                                                   std::uint64_t &_sizeOutput,
                                                                   std::uint8_t *&_dataOutput) const noexcept
{
    EMERGENCE_ASSERT (dataWritersCount == 0u);
    ++dataReadersCount;

    auto cursor = thirdPartyResourcesById.ReadPoint (&_id);
    const auto *resource = static_cast<const ThirdPartyResourceData *> (*cursor);

    if (!resource)
    {
        --dataReadersCount;
        return LoadingOperationResponse::NOT_FOUND;
    }

    FILE *file = std::fopen (EMERGENCE_BUILD_STRING (resource->source, "/", resource->relativePath), "rb");
    if (!file)
    {
        --dataReadersCount;
        return LoadingOperationResponse::NOT_FOUND;
    }

    fseek (file, 0u, SEEK_END);
    _sizeOutput = static_cast<std::uint64_t> (ftell (file));
    fseek (file, 0u, SEEK_SET);
    _dataOutput = static_cast<std::uint8_t *> (_allocator.Acquire (_sizeOutput, alignof (std::uint64_t)));

    if (fread (_dataOutput, 1u, _sizeOutput, file) != _sizeOutput)
    {
        --dataReadersCount;
        return LoadingOperationResponse::IO_ERROR;
    }

    fclose (file);
    --dataReadersCount;
    return LoadingOperationResponse::SUCCESSFUL;
}

ResourceProvider::ObjectRegistryCursor ResourceProvider::FindObjectsByType (
    const StandardLayout::Mapping &_type) const noexcept
{
    return {this, objectsByType.ReadPoint (&_type)};
}

SourceOperationResponse ResourceProvider::AddSourceFromIndex (Memory::UniqueString _path) noexcept
{
    std::ifstream input (EMERGENCE_BUILD_STRING (_path, "/", IndexFile::INDEX_FILE_NAME), std::ios::binary);
    if (!input)
    {
        return SourceOperationResponse::IO_ERROR;
    }

    IndexFile content;
    if (!Serialization::Binary::DeserializeObject (input, &content, IndexFile::Reflect ().mapping,
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
    if (!std::filesystem::exists (*_path))
    {
        EMERGENCE_LOG (ERROR, "ResourceProvider: Given source path \"", _path, "\" does not exist.");
        return SourceOperationResponse::NOT_FOUND;
    }

    SourceOperationResponse finalResponse = SourceOperationResponse::SUCCESSFUL;
    for (std::filesystem::recursive_directory_iterator iterator (
             EMERGENCE_BUILD_STRING (_path, "/"), std::filesystem::directory_options::follow_directory_symlink);
         iterator != std::filesystem::end (iterator); ++iterator)
    {
        const std::filesystem::directory_entry &entry = *iterator;
        if (!std::filesystem::is_regular_file (entry))
        {
            continue;
        }

        const Container::Utf8String relativePath =
            std::filesystem::relative (entry.path (), *_path)
                .generic_string<char, std::char_traits<char>, Memory::HeapSTD<char>> ();

        if (relativePath.ends_with (".bin"))
        {
            // Attempt to read type name.
            std::ifstream input (entry.path (), std::ios::binary);
            const Memory::UniqueString typeName = Serialization::Binary::DeserializeTypeName (input);

            if (!*typeName)
            {
                EMERGENCE_LOG (ERROR, "ResourceProvider: Failed to parse object type from \"", relativePath,
                               "\" of source \"", _path, "\".");
                finalResponse = SourceOperationResponse::IO_ERROR;
            }
            else if (SourceOperationResponse response =
                         AddObject (Memory::UniqueString (entry.path ().stem ().string ().c_str ()), typeName, _path,
                                    relativePath);
                     response != SourceOperationResponse::SUCCESSFUL)
            {
                finalResponse = response;
            }
        }
        else if (relativePath.ends_with (".yaml"))
        {
            // Attempt to read type name.
            std::ifstream input (entry.path ());
            const Memory::UniqueString typeName = Serialization::Yaml::DeserializeTypeName (input);

            if (!*typeName)
            {
                EMERGENCE_LOG (ERROR, "ResourceProvider: Failed to parse object type from \"", relativePath,
                               "\" of source \"", _path, "\".");
                finalResponse = SourceOperationResponse::IO_ERROR;
            }
            else if (SourceOperationResponse response =
                         AddObject (Memory::UniqueString (entry.path ().stem ().string ().c_str ()), typeName, _path,
                                    relativePath);
                     response != SourceOperationResponse::SUCCESSFUL)
            {
                finalResponse = response;
            }
        }
        else if (!relativePath.ends_with (IndexFile::INDEX_FILE_NAME))
        {
            if (SourceOperationResponse response = AddThirdPartyResource (
                    Memory::UniqueString (entry.path ().filename ().string ().c_str ()), _path, relativePath);
                response != SourceOperationResponse::SUCCESSFUL)
            {
                finalResponse = response;
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
    StandardLayout::Mapping type = objectTypesRegistry.Get (_typeName);
    if (!type)
    {
        EMERGENCE_LOG (ERROR, "ResourceProvider: Unknown object type \"", _typeName, "\"!");
        return SourceOperationResponse::UNKNOWN_TYPE;
    }

    ObjectResourceFormat format;
    if (_relativePath.ends_with (".bin"))
    {
        format = ObjectResourceFormat::BINARY;
    }
    else if (_relativePath.ends_with (".yaml"))
    {
        format = ObjectResourceFormat::YAML;
    }
    else
    {
        EMERGENCE_LOG (ERROR, "ResourceProvider: Unknown object format of file \"", _relativePath, "\"!");
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
    objectData->relativePath = _relativePath;
    objectData->format = format;
    return SourceOperationResponse::SUCCESSFUL;
}

SourceOperationResponse ResourceProvider::AddThirdPartyResource (Memory::UniqueString _id,
                                                                 Memory::UniqueString _source,
                                                                 const Container::Utf8String &_relativePath) noexcept
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
    resourceData->relativePath = _relativePath;
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

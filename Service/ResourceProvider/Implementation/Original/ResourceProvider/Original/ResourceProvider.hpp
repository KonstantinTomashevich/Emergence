#pragma once

#include <atomic>

#include <Container/String.hpp>

#include <ResourceProvider/ResourceProvider.hpp>

#include <RecordCollection/Collection.hpp>

namespace Emergence::ResourceProvider::Original
{
class ResourceProvider final
{
public:
    class ObjectRegistryCursor final
    {
    public:
        ObjectRegistryCursor (const ObjectRegistryCursor &_other) noexcept;

        ObjectRegistryCursor (ObjectRegistryCursor &&_other) noexcept;

        ~ObjectRegistryCursor () noexcept;

        [[nodiscard]] Memory::UniqueString operator* () const noexcept;

        ObjectRegistryCursor &operator++ () noexcept;

        EMERGENCE_DELETE_ASSIGNMENT (ObjectRegistryCursor);

    private:
        friend class ResourceProvider;

        ObjectRegistryCursor (const ResourceProvider *_owner,
                              RecordCollection::PointRepresentation::ReadCursor _cursor) noexcept;

        const ResourceProvider *owner;
        RecordCollection::PointRepresentation::ReadCursor cursor;
    };

    ResourceProvider (Container::MappingRegistry _objectTypesRegistry,
                      Container::MappingRegistry _patchableTypesRegistry) noexcept;

    ResourceProvider (const ResourceProvider &_other) = delete;

    ResourceProvider (ResourceProvider &&_other) = delete;

    ~ResourceProvider () noexcept;

    EMERGENCE_DELETE_ASSIGNMENT (ResourceProvider);

    const Container::MappingRegistry &GetObjectTypesRegistry () const noexcept;

    const Container::MappingRegistry &GetPatchableTypesRegistry () const noexcept;

    SourceOperationResponse AddSource (Memory::UniqueString _path) noexcept;

    SourceOperationResponse SaveSourceIndex (Memory::UniqueString _sourcePath) const noexcept;

    SourceOperationResponse RemoveSource (Memory::UniqueString _path) noexcept;

    LoadingOperationResponse LoadObject (const StandardLayout::Mapping &_type,
                                         Memory::UniqueString _id,
                                         void *_output) const noexcept;

    LoadingOperationResponse LoadThirdPartyResource (Memory::UniqueString _id,
                                                     Memory::Heap &_allocator,
                                                     uint64_t &_sizeOutput,
                                                     uint8_t *&_dataPointerOutput) const noexcept;

    ObjectRegistryCursor FindObjectsByType (const StandardLayout::Mapping &_type) const noexcept;

private:
    friend class ObjectRegistryCursor;

    SourceOperationResponse AddSourceFromIndex (Memory::UniqueString _path) noexcept;

    SourceOperationResponse AddSourceThroughScan (Memory::UniqueString _path) noexcept;

    SourceOperationResponse AddObject (Memory::UniqueString _id,
                                       Memory::UniqueString _typeName,
                                       Memory::UniqueString _source,
                                       const Container::String &_relativePath) noexcept;

    SourceOperationResponse AddThirdPartyResource (Memory::UniqueString _id,
                                                   Memory::UniqueString _source,
                                                   const Container::String &_relativePath) noexcept;

    bool ClearSource (Memory::UniqueString _path) noexcept;

    mutable std::atomic_unsigned_lock_free dataWritersCount = 0u;
    mutable std::atomic_unsigned_lock_free dataReadersCount = 0u;

    RecordCollection::Collection objects;
    mutable RecordCollection::PointRepresentation objectsById;
    mutable RecordCollection::PointRepresentation objectsByType;
    mutable RecordCollection::PointRepresentation objectsBySource;

    RecordCollection::Collection thirdPartyResources;
    mutable RecordCollection::PointRepresentation thirdPartyResourcesById;
    mutable RecordCollection::PointRepresentation thirdPartyResourcesBySource;

    Container::MappingRegistry objectTypesRegistry;
    Container::MappingRegistry patchableTypesRegistry;
};
} // namespace Emergence::ResourceProvider::Original

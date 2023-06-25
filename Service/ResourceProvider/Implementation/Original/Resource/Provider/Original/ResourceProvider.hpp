#pragma once

#include <atomic>

#include <Container/String.hpp>

#include <Resource/Provider/ResourceProvider.hpp>

#include <RecordCollection/Collection.hpp>

namespace Emergence::Resource::Provider::Original
{
class ResourceProvider final
{
public:
    class ObjectRegistryCursor final
    {
    public:
        ObjectRegistryCursor (const ObjectRegistryCursor &_other) noexcept = default;

        ObjectRegistryCursor (ObjectRegistryCursor &&_other) noexcept = default;

        ~ObjectRegistryCursor () noexcept = default;

        [[nodiscard]] Memory::UniqueString operator* () const noexcept;

        ObjectRegistryCursor &operator++ () noexcept;

        EMERGENCE_DELETE_ASSIGNMENT (ObjectRegistryCursor);

    private:
        friend class ResourceProvider;

        ObjectRegistryCursor (RecordCollection::PointRepresentation::ReadCursor _cursor) noexcept;

        RecordCollection::PointRepresentation::ReadCursor cursor;
    };

    class ThirdPartyRegistryCursor final
    {
    public:
        ThirdPartyRegistryCursor (const ThirdPartyRegistryCursor &_other) noexcept = default;

        ThirdPartyRegistryCursor (ThirdPartyRegistryCursor &&_other) noexcept = default;

        ~ThirdPartyRegistryCursor () noexcept = default;

        [[nodiscard]] Memory::UniqueString operator* () const noexcept;

        ThirdPartyRegistryCursor &operator++ () noexcept;

        EMERGENCE_DELETE_ASSIGNMENT (ThirdPartyRegistryCursor);

    private:
        friend class ResourceProvider;

        ThirdPartyRegistryCursor (RecordCollection::LinearRepresentation::AscendingReadCursor _cursor) noexcept;

        RecordCollection::LinearRepresentation::AscendingReadCursor cursor;
    };

    ResourceProvider (VirtualFileSystem::Context *_virtualFileSystemContext,
                      Container::MappingRegistry _objectTypesRegistry,
                      Container::MappingRegistry _patchableTypesRegistry) noexcept;

    ResourceProvider (const ResourceProvider &_other) = delete;

    ResourceProvider (ResourceProvider &&_other) = delete;

    ~ResourceProvider () noexcept = default;

    EMERGENCE_DELETE_ASSIGNMENT (ResourceProvider);

    const Container::MappingRegistry &GetObjectTypesRegistry () const noexcept;

    const Container::MappingRegistry &GetPatchableTypesRegistry () const noexcept;

    SourceOperationResponse AddSource (Memory::UniqueString _path) noexcept;

    SourceOperationResponse SaveSourceIndex (Memory::UniqueString _sourcePath,
                                             const VirtualFileSystem::Entry &_output) const noexcept;

    SourceOperationResponse RemoveSource (Memory::UniqueString _path) noexcept;

    LoadingOperationResponse LoadObject (const StandardLayout::Mapping &_type,
                                         Memory::UniqueString _id,
                                         void *_output) const noexcept;

    LoadingOperationResponse LoadThirdPartyResource (Memory::UniqueString _id,
                                                     Memory::Heap &_allocator,
                                                     std::uint64_t &_sizeOutput,
                                                     std::uint8_t *&_dataOutput) const noexcept;

    [[nodiscard]] ObjectRegistryCursor FindObjectsByType (const StandardLayout::Mapping &_type) const noexcept;

    [[nodiscard]] ThirdPartyRegistryCursor VisitAllThirdParty () const noexcept;

    [[nodiscard]] ObjectFormat GetObjectFormat (const StandardLayout::Mapping &_type,
                                                Memory::UniqueString _id) const noexcept;

    [[nodiscard]] VirtualFileSystem::Entry GetObjectEntry (const StandardLayout::Mapping &_type,
                                                           Memory::UniqueString _id) const noexcept;

    [[nodiscard]] VirtualFileSystem::Entry GetThirdPartyEntry (Memory::UniqueString _id) const noexcept;

private:
    friend class ObjectRegistryCursor;

    SourceOperationResponse AddSourceFromIndex (const VirtualFileSystem::Entry &_indexFile,
                                                Memory::UniqueString _path) noexcept;

    SourceOperationResponse AddSourceThroughScan (Memory::UniqueString _path) noexcept;

    SourceOperationResponse AddObject (Memory::UniqueString _id,
                                       Memory::UniqueString _typeName,
                                       Memory::UniqueString _source,
                                       const Container::Utf8String &_relativePath) noexcept;

    SourceOperationResponse AddObject (Memory::UniqueString _id,
                                       Memory::UniqueString _typeName,
                                       Memory::UniqueString _source,
                                       const VirtualFileSystem::Entry &_entry) noexcept;

    SourceOperationResponse AddThirdPartyResource (Memory::UniqueString _id,
                                                   Memory::UniqueString _source,
                                                   const Container::Utf8String &_relativePath) noexcept;

    SourceOperationResponse AddThirdPartyResource (Memory::UniqueString _id,
                                                   Memory::UniqueString _source,
                                                   const VirtualFileSystem::Entry &_entry) noexcept;

    bool ClearSource (Memory::UniqueString _path) noexcept;

    VirtualFileSystem::Context *virtualFileSystemContext;

    RecordCollection::Collection objects;
    mutable RecordCollection::PointRepresentation objectsById;
    mutable RecordCollection::PointRepresentation objectsByType;
    mutable RecordCollection::PointRepresentation objectsBySource;

    RecordCollection::Collection thirdPartyResources;
    mutable RecordCollection::LinearRepresentation thirdPartyResourcesOrderedById;
    mutable RecordCollection::PointRepresentation thirdPartyResourcesById;
    mutable RecordCollection::PointRepresentation thirdPartyResourcesBySource;

    Container::MappingRegistry objectTypesRegistry;
    Container::MappingRegistry patchableTypesRegistry;
};
} // namespace Emergence::Resource::Provider::Original

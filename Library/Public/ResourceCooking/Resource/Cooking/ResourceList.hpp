#pragma once

#include <Container/Optional.hpp>

#include <RecordCollection/Collection.hpp>

#include <Resource/Provider/ObjectFormat.hpp>

#include <VirtualFileSystem/Entry.hpp>

namespace Emergence::Resource::Cooking
{
struct ObjectData final
{
    Memory::UniqueString id;
    StandardLayout::Mapping type;
    VirtualFileSystem::Entry entry;
    Provider::ObjectFormat format = Provider::ObjectFormat::BINARY;

    struct Reflection final
    {
        StandardLayout::FieldId id;
        StandardLayout::FieldId typeNumber;
        StandardLayout::FieldId format;
        StandardLayout::Mapping mapping;
    };

    static const Reflection &Reflect () noexcept;
};

struct ThirdPartyData final
{
    Memory::UniqueString id;
    VirtualFileSystem::Entry entry;

    struct Reflection final
    {
        StandardLayout::FieldId id;
        StandardLayout::Mapping mapping;
    };

    static const Reflection &Reflect () noexcept;
};

class ResourceList final
{
public:
    class AllObjectsCursor final
    {
    public:
        AllObjectsCursor (const AllObjectsCursor &_other) noexcept = default;

        AllObjectsCursor (AllObjectsCursor &&_other) noexcept = default;

        ~AllObjectsCursor () noexcept = default;

        [[nodiscard]] Container::Optional<ObjectData> operator* () const noexcept;

        AllObjectsCursor &operator++ () noexcept;

        EMERGENCE_DELETE_ASSIGNMENT (AllObjectsCursor);

    private:
        friend class ResourceList;

        AllObjectsCursor (RecordCollection::LinearRepresentation::AscendingReadCursor _cursor) noexcept;

        RecordCollection::LinearRepresentation::AscendingReadCursor cursor;
    };

    class AllObjectsOfTypeCursor final
    {
    public:
        AllObjectsOfTypeCursor (const AllObjectsOfTypeCursor &_other) noexcept = default;

        AllObjectsOfTypeCursor (AllObjectsOfTypeCursor &&_other) noexcept = default;

        ~AllObjectsOfTypeCursor () noexcept = default;

        [[nodiscard]] Container::Optional<ObjectData> operator* () const noexcept;

        AllObjectsOfTypeCursor &operator++ () noexcept;

        EMERGENCE_DELETE_ASSIGNMENT (AllObjectsOfTypeCursor);

    private:
        friend class ResourceList;

        AllObjectsOfTypeCursor (RecordCollection::PointRepresentation::ReadCursor _cursor) noexcept;

        RecordCollection::PointRepresentation::ReadCursor cursor;
    };

    class AllThirdPartyCursor final
    {
    public:
        AllThirdPartyCursor (const AllThirdPartyCursor &_other) noexcept = default;

        AllThirdPartyCursor (AllThirdPartyCursor &&_other) noexcept = default;

        ~AllThirdPartyCursor () noexcept = default;

        [[nodiscard]] Container::Optional<ThirdPartyData> operator* () const noexcept;

        AllThirdPartyCursor &operator++ () noexcept;

        EMERGENCE_DELETE_ASSIGNMENT (AllThirdPartyCursor);

    private:
        friend class ResourceList;

        AllThirdPartyCursor (RecordCollection::LinearRepresentation::AscendingReadCursor _cursor) noexcept;

        RecordCollection::LinearRepresentation::AscendingReadCursor cursor;
    };

    ResourceList () noexcept;

    ResourceList (const ResourceList &_other) = delete;

    ResourceList (ResourceList &&_other) = delete;

    ~ResourceList () noexcept = default;

    void AddObject (const ObjectData &_object) noexcept;

    void AddThirdParty (const ThirdPartyData &_thirdParty) noexcept;

    AllObjectsCursor VisitAllObjects () const noexcept;

    AllObjectsOfTypeCursor VisitAllObjectsOfType (const StandardLayout::Mapping &_mapping) const noexcept;

    AllThirdPartyCursor VisitAllThirdParty () const noexcept;

    Container::Optional<ObjectData> QueryObject (const Memory::UniqueString &_id) const noexcept;

    Container::Optional<ThirdPartyData> QueryThirdParty (const Memory::UniqueString &_id) const noexcept;

    void Clear () noexcept;

    EMERGENCE_DELETE_ASSIGNMENT (ResourceList);

private:
    RecordCollection::Collection objects;
    mutable RecordCollection::LinearRepresentation allObjects;
    mutable RecordCollection::PointRepresentation objectsByType;
    mutable RecordCollection::PointRepresentation objectsById;

    RecordCollection::Collection thirdParty;
    mutable RecordCollection::LinearRepresentation allThirdParty;
    mutable RecordCollection::PointRepresentation thirdPartyById;
};
} // namespace Emergence::Resource::Cooking

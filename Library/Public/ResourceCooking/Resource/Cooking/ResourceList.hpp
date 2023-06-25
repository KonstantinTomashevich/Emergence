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
    class AllObjectsReadCursor final
    {
    public:
        AllObjectsReadCursor (const AllObjectsReadCursor &_other) noexcept = default;

        AllObjectsReadCursor (AllObjectsReadCursor &&_other) noexcept = default;

        ~AllObjectsReadCursor () noexcept = default;

        [[nodiscard]] const ObjectData *operator* () const noexcept;

        AllObjectsReadCursor &operator++ () noexcept;

        EMERGENCE_DELETE_ASSIGNMENT (AllObjectsReadCursor);

    private:
        friend class ResourceList;

        AllObjectsReadCursor (RecordCollection::LinearRepresentation::AscendingReadCursor _cursor) noexcept;

        RecordCollection::LinearRepresentation::AscendingReadCursor cursor;
    };

    class AllObjectsEditCursor final
    {
    public:
        AllObjectsEditCursor (const AllObjectsEditCursor &_other) = delete;

        AllObjectsEditCursor (AllObjectsEditCursor &&_other) noexcept = default;

        ~AllObjectsEditCursor () noexcept = default;

        [[nodiscard]] ObjectData *operator* () noexcept;

        AllObjectsEditCursor &operator++ () noexcept;

        AllObjectsEditCursor &operator~() noexcept;

        EMERGENCE_DELETE_ASSIGNMENT (AllObjectsEditCursor);

    private:
        friend class ResourceList;

        AllObjectsEditCursor (RecordCollection::LinearRepresentation::AscendingEditCursor _cursor) noexcept;

        RecordCollection::LinearRepresentation::AscendingEditCursor cursor;
    };

    class AllObjectsOfTypeReadCursor final
    {
    public:
        AllObjectsOfTypeReadCursor (const AllObjectsOfTypeReadCursor &_other) noexcept = default;

        AllObjectsOfTypeReadCursor (AllObjectsOfTypeReadCursor &&_other) noexcept = default;

        ~AllObjectsOfTypeReadCursor () noexcept = default;

        [[nodiscard]] const ObjectData *operator* () const noexcept;

        AllObjectsOfTypeReadCursor &operator++ () noexcept;

        EMERGENCE_DELETE_ASSIGNMENT (AllObjectsOfTypeReadCursor);

    private:
        friend class ResourceList;

        AllObjectsOfTypeReadCursor (RecordCollection::PointRepresentation::ReadCursor _cursor) noexcept;

        RecordCollection::PointRepresentation::ReadCursor cursor;
    };

    class AllObjectsOfTypeEditCursor final
    {
    public:
        AllObjectsOfTypeEditCursor (const AllObjectsOfTypeEditCursor &_other) = delete;

        AllObjectsOfTypeEditCursor (AllObjectsOfTypeEditCursor &&_other) noexcept = default;

        ~AllObjectsOfTypeEditCursor () noexcept = default;

        [[nodiscard]] ObjectData *operator* () noexcept;

        AllObjectsOfTypeEditCursor &operator++ () noexcept;

        AllObjectsOfTypeEditCursor &operator~() noexcept;

        EMERGENCE_DELETE_ASSIGNMENT (AllObjectsOfTypeEditCursor);

    private:
        friend class ResourceList;

        AllObjectsOfTypeEditCursor (RecordCollection::PointRepresentation::EditCursor _cursor) noexcept;

        RecordCollection::PointRepresentation::EditCursor cursor;
    };

    class AllThirdPartyReadCursor final
    {
    public:
        AllThirdPartyReadCursor (const AllThirdPartyReadCursor &_other) noexcept = default;

        AllThirdPartyReadCursor (AllThirdPartyReadCursor &&_other) noexcept = default;

        ~AllThirdPartyReadCursor () noexcept = default;

        [[nodiscard]] const ThirdPartyData *operator* () const noexcept;

        AllThirdPartyReadCursor &operator++ () noexcept;

        EMERGENCE_DELETE_ASSIGNMENT (AllThirdPartyReadCursor);

    private:
        friend class ResourceList;

        AllThirdPartyReadCursor (RecordCollection::LinearRepresentation::AscendingReadCursor _cursor) noexcept;

        RecordCollection::LinearRepresentation::AscendingReadCursor cursor;
    };

    class AllThirdPartyEditCursor final
    {
    public:
        AllThirdPartyEditCursor (const AllThirdPartyEditCursor &_other) = delete;

        AllThirdPartyEditCursor (AllThirdPartyEditCursor &&_other) noexcept = default;

        ~AllThirdPartyEditCursor () noexcept = default;

        [[nodiscard]] ThirdPartyData *operator* () noexcept;

        AllThirdPartyEditCursor &operator++ () noexcept;

        AllThirdPartyEditCursor &operator~() noexcept;

        EMERGENCE_DELETE_ASSIGNMENT (AllThirdPartyEditCursor);

    private:
        friend class ResourceList;

        AllThirdPartyEditCursor (RecordCollection::LinearRepresentation::AscendingEditCursor _cursor) noexcept;

        RecordCollection::LinearRepresentation::AscendingEditCursor cursor;
    };

    ResourceList () noexcept;

    ResourceList (const ResourceList &_other) = delete;

    ResourceList (ResourceList &&_other) = delete;

    ~ResourceList () noexcept = default;

    void AddObject (const ObjectData &_object) noexcept;

    void AddThirdParty (const ThirdPartyData &_thirdParty) noexcept;

    Container::Optional<ObjectData> QueryObject (Memory::UniqueString _id) const noexcept;

    Container::Optional<ThirdPartyData> QueryThirdParty (Memory::UniqueString _id) const noexcept;

    void RemoveObject (Memory::UniqueString _id) noexcept;

    void RemoveThirdParty (Memory::UniqueString _id) noexcept;

    AllObjectsReadCursor ReadAllObjects () const noexcept;

    AllObjectsEditCursor EditAllObjects () noexcept;

    AllObjectsOfTypeReadCursor ReadAllObjectsOfType (const StandardLayout::Mapping &_mapping) const noexcept;

    AllObjectsOfTypeEditCursor EditAllObjectsOfType (const StandardLayout::Mapping &_mapping) noexcept;

    AllThirdPartyReadCursor ReadAllThirdParty () const noexcept;

    AllThirdPartyEditCursor EditAllThirdParty () noexcept;

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

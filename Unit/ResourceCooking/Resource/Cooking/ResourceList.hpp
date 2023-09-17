#pragma once

#include <ResourceCookingApi.hpp>

#include <Container/Optional.hpp>

#include <RecordCollection/Collection.hpp>

#include <Resource/Provider/ObjectFormat.hpp>

#include <VirtualFileSystem/Entry.hpp>

namespace Emergence::Resource::Cooking
{
/// \brief Describes a reflection-driven resource object instance.
struct ResourceCookingApi ObjectData final
{
    /// \brief Unique identifier of this object. Usually it is a file name without last extension.
    Memory::UniqueString id;

    /// \brief Mapping that describes contents of this object.
    StandardLayout::Mapping type;

    /// \brief Entry that points to where this object is stored.
    VirtualFileSystem::Entry entry;

    /// \brief Format in which this object is stored.
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

/// \brief Describes a third-party (not backed by reflection) resource instance.
struct ResourceCookingApi ThirdPartyData final
{
    /// \brief Unique identifier of this resource. Usually it is a file name including extensions.
    Memory::UniqueString id;

    /// \brief Entry that points to where this resource is stored.
    VirtualFileSystem::Entry entry;

    struct Reflection final
    {
        StandardLayout::FieldId id;
        StandardLayout::Mapping mapping;
    };

    static const Reflection &Reflect () noexcept;
};

/// \brief Stores list of resources, both reflection-driven and third-party, that are currently being processed.
/// \details Goal of this class is to store resource list in an efficient manner backed by indexing.
///
/// \par Access rules and thread safety
/// \parblock
/// This class follows readers-writers pattern: calling const methods and using read cursors is thread safe unless any
/// non-const method is being executed or edit cursor exists. Non-const methods and edit cursors usage is not thread
/// safe. There are no inbuilt shared locking: user is responsible for taking care of thread safety if it is needed.
/// \endparblock
class ResourceCookingApi ResourceList final
{
public:
    /// \brief Provides read access to all reflection-driven resource objects.
    class ResourceCookingApi AllObjectsReadCursor final
    {
    public:
        AllObjectsReadCursor (const AllObjectsReadCursor &_other) noexcept = default;

        AllObjectsReadCursor (AllObjectsReadCursor &&_other) noexcept = default;

        ~AllObjectsReadCursor () noexcept = default;

        /// \return Pointer to current object data or nullptr if there is no more objects.
        [[nodiscard]] const ObjectData *operator* () const noexcept;

        /// \brief Moves cursor to the next object.
        /// \invariant Cursor is not pointing to the end.
        AllObjectsReadCursor &operator++ () noexcept;

        EMERGENCE_DELETE_ASSIGNMENT (AllObjectsReadCursor);

    private:
        friend class ResourceList;

        AllObjectsReadCursor (RecordCollection::LinearRepresentation::AscendingReadCursor _cursor) noexcept;

        RecordCollection::LinearRepresentation::AscendingReadCursor cursor;
    };

    /// \brief Provides edit access to all reflection-driven resource objects.
    class ResourceCookingApi AllObjectsEditCursor final
    {
    public:
        AllObjectsEditCursor (const AllObjectsEditCursor &_other) = delete;

        AllObjectsEditCursor (AllObjectsEditCursor &&_other) noexcept = default;

        ~AllObjectsEditCursor () noexcept = default;

        /// \return Pointer to current object data or nullptr if there is no more objects.
        [[nodiscard]] ObjectData *operator* () noexcept;

        /// \brief Moves cursor to the next object.
        /// \invariant Cursor is not pointing to the end.
        AllObjectsEditCursor &operator++ () noexcept;

        /// \brief Deletes object under the cursor and moves to the next one.
        /// \invariant Cursor is not pointing to the end.
        AllObjectsEditCursor &operator~() noexcept;

        EMERGENCE_DELETE_ASSIGNMENT (AllObjectsEditCursor);

    private:
        friend class ResourceList;

        AllObjectsEditCursor (RecordCollection::LinearRepresentation::AscendingEditCursor _cursor) noexcept;

        RecordCollection::LinearRepresentation::AscendingEditCursor cursor;
    };

    /// \brief Provides read access to all reflection-driven resource objects of requested type.
    class ResourceCookingApi AllObjectsOfTypeReadCursor final
    {
    public:
        AllObjectsOfTypeReadCursor (const AllObjectsOfTypeReadCursor &_other) noexcept = default;

        AllObjectsOfTypeReadCursor (AllObjectsOfTypeReadCursor &&_other) noexcept = default;

        ~AllObjectsOfTypeReadCursor () noexcept = default;

        /// \return Pointer to current object data or nullptr if there is no more objects.
        [[nodiscard]] const ObjectData *operator* () const noexcept;

        /// \brief Moves cursor to the next object.
        /// \invariant Cursor is not pointing to the end.
        AllObjectsOfTypeReadCursor &operator++ () noexcept;

        EMERGENCE_DELETE_ASSIGNMENT (AllObjectsOfTypeReadCursor);

    private:
        friend class ResourceList;

        AllObjectsOfTypeReadCursor (RecordCollection::PointRepresentation::ReadCursor _cursor) noexcept;

        RecordCollection::PointRepresentation::ReadCursor cursor;
    };

    /// \brief Provides edit access to all reflection-driven resource objects of requested type.
    class ResourceCookingApi AllObjectsOfTypeEditCursor final
    {
    public:
        AllObjectsOfTypeEditCursor (const AllObjectsOfTypeEditCursor &_other) = delete;

        AllObjectsOfTypeEditCursor (AllObjectsOfTypeEditCursor &&_other) noexcept = default;

        ~AllObjectsOfTypeEditCursor () noexcept = default;

        /// \return Pointer to current object data or nullptr if there is no more objects.
        [[nodiscard]] ObjectData *operator* () noexcept;

        /// \brief Moves cursor to the next object.
        /// \invariant Cursor is not pointing to the end.
        AllObjectsOfTypeEditCursor &operator++ () noexcept;

        /// \brief Deletes object under the cursor and moves to the next one.
        /// \invariant Cursor is not pointing to the end.
        AllObjectsOfTypeEditCursor &operator~() noexcept;

        EMERGENCE_DELETE_ASSIGNMENT (AllObjectsOfTypeEditCursor);

    private:
        friend class ResourceList;

        AllObjectsOfTypeEditCursor (RecordCollection::PointRepresentation::EditCursor _cursor) noexcept;

        RecordCollection::PointRepresentation::EditCursor cursor;
    };

    /// \brief Provides read access to all third-party resources.
    class ResourceCookingApi AllThirdPartyReadCursor final
    {
    public:
        AllThirdPartyReadCursor (const AllThirdPartyReadCursor &_other) noexcept = default;

        AllThirdPartyReadCursor (AllThirdPartyReadCursor &&_other) noexcept = default;

        ~AllThirdPartyReadCursor () noexcept = default;

        /// \return Pointer to current resource data or nullptr if there is no more resources.
        [[nodiscard]] const ThirdPartyData *operator* () const noexcept;

        /// \brief Moves cursor to the next resource.
        /// \invariant Cursor is not pointing to the end.
        AllThirdPartyReadCursor &operator++ () noexcept;

        EMERGENCE_DELETE_ASSIGNMENT (AllThirdPartyReadCursor);

    private:
        friend class ResourceList;

        AllThirdPartyReadCursor (RecordCollection::LinearRepresentation::AscendingReadCursor _cursor) noexcept;

        RecordCollection::LinearRepresentation::AscendingReadCursor cursor;
    };

    /// \brief Provides edit access to all third-party resources.
    class ResourceCookingApi AllThirdPartyEditCursor final
    {
    public:
        AllThirdPartyEditCursor (const AllThirdPartyEditCursor &_other) = delete;

        AllThirdPartyEditCursor (AllThirdPartyEditCursor &&_other) noexcept = default;

        ~AllThirdPartyEditCursor () noexcept = default;

        /// \return Pointer to current resource data or nullptr if there is no more resources.
        [[nodiscard]] ThirdPartyData *operator* () noexcept;

        /// \brief Moves cursor to the next resource.
        /// \invariant Cursor is not pointing to the end.
        AllThirdPartyEditCursor &operator++ () noexcept;

        /// \brief Deletes resource under the cursor and moves to the next one.
        /// \invariant Cursor is not pointing to the end.
        AllThirdPartyEditCursor &operator~() noexcept;

        EMERGENCE_DELETE_ASSIGNMENT (AllThirdPartyEditCursor);

    private:
        friend class ResourceList;

        AllThirdPartyEditCursor (RecordCollection::LinearRepresentation::AscendingEditCursor _cursor) noexcept;

        RecordCollection::LinearRepresentation::AscendingEditCursor cursor;
    };

    /// \brief Constructs empty resource list.
    ResourceList () noexcept;

    ResourceList (const ResourceList &_other) = delete;

    ResourceList (ResourceList &&_other) = delete;

    ~ResourceList () noexcept = default;

    /// \brief Adds new reflection-driven resource object to the list.
    /// \invariant User is responsible for preserving id uniqueness.
    void AddObject (const ObjectData &_object) noexcept;

    /// \brief Adds new third-party resource to the list.
    /// \invariant User is responsible for preserving id uniqueness.
    void AddThirdParty (const ThirdPartyData &_thirdParty) noexcept;

    /// \return Object with given id or nullopt if there is no such object.
    /// \details We return optional with copy instead of pointer,
    ///          otherwise it would be impossible to preserve read-write scope.
    Container::Optional<ObjectData> QueryObject (Memory::UniqueString _id) const noexcept;

    /// \return Resource with given id or nullopt if there is no such resource.
    /// \details We return optional with copy instead of pointer,
    ///          otherwise it would be impossible to preserve read-write scope.
    Container::Optional<ThirdPartyData> QueryThirdParty (Memory::UniqueString _id) const noexcept;

    /// \brief Removes reflection-driven resource object with given id if it exists.
    void RemoveObject (Memory::UniqueString _id) noexcept;

    /// \brief Removes third-party resource with given id if it exists.
    void RemoveThirdParty (Memory::UniqueString _id) noexcept;

    /// \return Cursor with read access to all reflection-driven resource objects.
    AllObjectsReadCursor ReadAllObjects () const noexcept;

    /// \return Cursor with edit access to all reflection-driven resource objects.
    AllObjectsEditCursor EditAllObjects () noexcept;

    /// \return Cursor with read access to all reflection-driven resource objects of requested type.
    AllObjectsOfTypeReadCursor ReadAllObjectsOfType (const StandardLayout::Mapping &_mapping) const noexcept;

    /// \return Cursor with edit access to all reflection-driven resource objects of requested type.
    AllObjectsOfTypeEditCursor EditAllObjectsOfType (const StandardLayout::Mapping &_mapping) noexcept;

    /// \return Cursor with read access to all third-party resources.
    AllThirdPartyReadCursor ReadAllThirdParty () const noexcept;

    /// \return Cursor with edit access to all third-party resources.
    AllThirdPartyEditCursor EditAllThirdParty () noexcept;

    /// \brief Removes everything from the list, making it empty.
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

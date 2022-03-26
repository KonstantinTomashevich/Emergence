#pragma once

#include <API/Common/Cursor.hpp>
#include <API/Common/ImplementationBinding.hpp>
#include <API/Common/Iterator.hpp>
#include <API/Common/Shortcuts.hpp>

#include <Container/Vector.hpp>

#include <StandardLayout/Mapping.hpp>

#include <RecordCollection/LinearRepresentation.hpp>
#include <RecordCollection/PointRepresentation.hpp>
#include <RecordCollection/VolumetricRepresentation.hpp>

namespace Emergence::RecordCollection
{
/// \brief Stores records of the same type and provides fast lookup using representations.
class Collection final
{
public:
    /// \brief Wraps allocation and insertion of several records into one transaction.
    class Allocator final
    {
    public:
        Allocator (const Allocator &_other) = delete;

        Allocator (Allocator &&_other) noexcept;

        /// \brief Ends transaction. Inserts all allocated records, that are not inserted yet.
        ~Allocator () noexcept;

        /// \brief Allocates new record for insertion.
        /// \warning If record type has no default constructor, allocated record will not be initialized.
        /// \invariant Previous allocated recorded must be initialized and ready for insertion.
        void *Allocate () noexcept;

        /// Assigning allocators looks counter intuitive.
        EMERGENCE_DELETE_ASSIGNMENT (Allocator);

    private:
        /// Collection constructs allocators.
        friend class Collection;

        EMERGENCE_BIND_IMPLEMENTATION_INPLACE (sizeof (uintptr_t) * 2u);

        explicit Allocator (std::array<uint8_t, DATA_MAX_SIZE> *_data) noexcept;
    };

    /// \brief Allows iteration over Collection linear representations.
    ///
    /// \warning Collection::CreateLinearRepresentation invalidates these iterators.
    class LinearRepresentationIterator final
    {
    public:
        EMERGENCE_BIDIRECTIONAL_ITERATOR_OPERATIONS (LinearRepresentationIterator, LinearRepresentation);

    private:
        /// Collection constructs iterators for linear representations.
        friend class Collection;

        EMERGENCE_BIND_IMPLEMENTATION_INPLACE (sizeof (uintptr_t));

        explicit LinearRepresentationIterator (const std::array<uint8_t, DATA_MAX_SIZE> *_data) noexcept;
    };

    /// \brief Allows iteration over Collection point representations.
    ///
    /// \warning Collection::CreatePointRepresentation invalidates these iterators.
    class PointRepresentationIterator final
    {
    public:
        EMERGENCE_BIDIRECTIONAL_ITERATOR_OPERATIONS (PointRepresentationIterator, PointRepresentation);

    private:
        /// Collection constructs iterators for point representations.
        friend class Collection;

        EMERGENCE_BIND_IMPLEMENTATION_INPLACE (sizeof (uintptr_t));

        explicit PointRepresentationIterator (const std::array<uint8_t, DATA_MAX_SIZE> *_data) noexcept;
    };

    /// \brief Allows iteration over Collection volumetric representations.
    ///
    /// \warning Collection::CreateVolumetricRepresentation invalidates these iterators.
    class VolumetricRepresentationIterator final
    {
    public:
        EMERGENCE_BIDIRECTIONAL_ITERATOR_OPERATIONS (VolumetricRepresentationIterator, VolumetricRepresentation);

    private:
        /// Collection constructs iterators for volumetric representations.
        friend class Collection;

        EMERGENCE_BIND_IMPLEMENTATION_INPLACE (sizeof (uintptr_t));

        explicit VolumetricRepresentationIterator (const std::array<uint8_t, DATA_MAX_SIZE> *_data) noexcept;
    };

    /// \brief Describes one of the VolumetricRepresentation dimensions, used for VolumetricRepresentation creation.
    struct DimensionDescriptor
    {
        /// \brief Pointer to minimum possible value of #minBorderField.
        ///
        /// \details Values, that are less than this value will be processed as this value.
        ///          Guarantied to be copied during representation creation, therefore can point to stack memory.
        const void *globalMinBorder;

        /// \brief Id of field, that holds record minimum border value for this dimension.
        ///
        /// \invariant Field archetype is FieldArchetype::INT, FieldArchetype::UINT or FieldArchetype::FLOAT.
        StandardLayout::FieldId minBorderField;

        /// \brief Pointer to maximum possible value of #maxBorderField.
        ///
        /// \details Values, that are greater than this value will be processed as this value.
        ///          Guarantied to be copied during representation creation, therefore can point to stack memory.
        const void *globalMaxBorder;

        /// \brief Id of field, that holds record maximum border value for this dimension.
        ///
        /// \invariant Field archetype is FieldArchetype::INT, FieldArchetype::UINT or FieldArchetype::FLOAT.
        StandardLayout::FieldId maxBorderField;
    };

    /// \brief Construct collection for records of the type, which fields are described in given _typeMapping.
    explicit Collection (StandardLayout::Mapping _typeMapping);

    /// Collections are designed to store lots of records, therefore it's not optimal to copy such collections.
    Collection (const Collection &_other) = delete;

    Collection (Collection &&_other) noexcept;

    ~Collection () noexcept;

    /// \brief Starts allocation transaction.
    /// \invariant There is no active allocation transactions in this collection and cursors in its representations.
    Allocator AllocateAndInsert () noexcept;

    /// \brief Adds LinearRepresentation to Collection, which sorts records by value of given _keyField.
    /// \invariant There is no active allocation transactions in this collection and cursors in its representations.
    [[nodiscard]] LinearRepresentation CreateLinearRepresentation (StandardLayout::FieldId _keyField) noexcept;

    /// \brief Adds PointRepresentation to Collection, that uses given _keyFields as point position.
    /// \invariant There is no active allocation transactions in this collection and cursors in its representations.
    [[nodiscard]] PointRepresentation CreatePointRepresentation (
        const Container::Vector<StandardLayout::FieldId> &_keyFields) noexcept;

    /// \brief Adds VolumetricRepresentation to Collection, that uses given _dimensions.
    /// \invariant There is no active allocation transactions in this collection and cursors in its representations.
    /// \invariant All border fields for all dimensions should have same archetype and same size.
    [[nodiscard]] VolumetricRepresentation CreateVolumetricRepresentation (
        const Container::Vector<DimensionDescriptor> &_dimensions) noexcept;

    /// \return Object type mapping for objects, that are stored in this collection.
    [[nodiscard]] const StandardLayout::Mapping &GetTypeMapping () const noexcept;

    /// \return Iterator, that points to beginning of linear representations range.
    [[nodiscard]] LinearRepresentationIterator LinearRepresentationBegin () const noexcept;

    /// \return Iterator, that points to ending of linear representations range.
    [[nodiscard]] LinearRepresentationIterator LinearRepresentationEnd () const noexcept;

    /// \return Iterator, that points to beginning of point representations range.
    [[nodiscard]] PointRepresentationIterator PointRepresentationBegin () const noexcept;

    /// \return Iterator, that points to ending of point representations range.
    [[nodiscard]] PointRepresentationIterator PointRepresentationEnd () const noexcept;

    /// \return Iterator, that points to beginning of volumetric representations range.
    [[nodiscard]] VolumetricRepresentationIterator VolumetricRepresentationBegin () const noexcept;

    /// \return Iterator, that points to ending of volumetric representations range.
    [[nodiscard]] VolumetricRepresentationIterator VolumetricRepresentationEnd () const noexcept;

    /// \brief Allows to lift `no edit cursors` invariant for read cursors and `no read cursors` for edit cursors.
    /// \details By lifting these requirements user states that these invariants are true:
    ///          - There is no multi thread access to Collection during this period.
    ///          - Key fields of read cursors are not modified by edit cursors.
    ///          - New records are not allocated.
    ///          - Edit cursor is not deleting records.
    ///          Breaking these invariants results in undefined behaviour.
    ///
    ///          In most cases user should avoid entering unsafe read mode, but some rare tasks, that are usually
    ///          connected with hierarchical access, can not be solved without this tricky approach.
    void SetUnsafeReadAllowed (bool _allowed) noexcept;

    /// Collections are designed to store lots of records, therefore it's not optimal to copy assign such collections.
    Collection &operator= (const Collection &_other) = delete;

    Collection &operator= (Collection &&_other) noexcept;

private:
    EMERGENCE_BIND_IMPLEMENTATION_INPLACE (sizeof (uintptr_t) * 2u);
};
} // namespace Emergence::RecordCollection

EMERGENCE_MEMORY_DEFAULT_ALLOCATION_GROUP (Emergence::RecordCollection::Collection::DimensionDescriptor)

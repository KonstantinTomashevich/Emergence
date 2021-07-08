#pragma once

#include <vector>

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
        /// \warning Record type is unknown during compile time, therefore
        ///          appropriate constructor should be called after allocation.
        /// \invariant Previous allocated recorded must be initialized and ready for insertion.
        void *Allocate () noexcept;

        /// Assigning allocators looks counter intuitive.
        Allocator &operator = (const Allocator &_other) = delete;

        /// Assigning allocators looks counter intuitive.
        Allocator &operator = (Allocator &&_other) = delete;

    private:
        /// Collection constructs allocators.
        friend class Collection;

        static constexpr std::size_t DATA_MAX_SIZE = sizeof (uintptr_t);

        explicit Allocator (const std::array <uint8_t, DATA_MAX_SIZE> *_data) noexcept;

        /// \brief Allocator implementation-specific data.
        std::array <uint8_t, DATA_MAX_SIZE> data;
    };

    /// \brief Allows iteration over Collection linear representations.
    ///
    /// \warning Collection::CreateLinearRepresentation invalidates these iterators.
    class LinearRepresentationIterator final
    {
    public:
        ~LinearRepresentationIterator () noexcept;

        /// \return LinearRepresentation, to which iterator points.
        /// \invariant Inside valid bounds, but not in the ending.
        LinearRepresentation operator * () const noexcept;

        /// \brief Move to next field.
        /// \invariant Inside valid bounds, but not in the ending.
        LinearRepresentationIterator &operator ++ () noexcept;

        /// \brief Move to next field.
        /// \return Unchanged instance of iterator.
        /// \invariant Inside valid bounds, but not in the ending.
        LinearRepresentationIterator operator ++ (int) noexcept;

        /// \brief Move to previous field.
        /// \invariant Inside valid bounds, but not in the beginning.
        LinearRepresentationIterator &operator -- () noexcept;

        /// \brief Move to previous field.
        /// \return Unchanged instance of iterator.
        /// \invariant Inside valid bounds, but not in the beginning.
        LinearRepresentationIterator operator -- (int) noexcept;

        bool operator == (const LinearRepresentationIterator &_other) const noexcept;

        bool operator != (const LinearRepresentationIterator &_other) const noexcept;

    private:
        /// Collection constructs iterators for linear representations.
        friend class Collection;

        static constexpr std::size_t DATA_MAX_SIZE = sizeof (uintptr_t);

        explicit LinearRepresentationIterator (const std::array <uint8_t, DATA_MAX_SIZE> *_data) noexcept;

        /// \brief Iterator implementation-specific data.
        std::array <uint8_t, DATA_MAX_SIZE> data;
    };

    /// \brief Allows iteration over Collection point representations.
    ///
    /// \warning Collection::CreatePointRepresentation invalidates these iterators.
    class PointRepresentationIterator final
    {
    public:
        ~PointRepresentationIterator () noexcept;

        /// \return PointRepresentation, to which iterator points.
        /// \invariant Inside valid bounds, but not in the ending.
        PointRepresentation operator * () const noexcept;

        /// \brief Move to next field.
        /// \invariant Inside valid bounds, but not in the ending.
        PointRepresentationIterator &operator ++ () noexcept;

        /// \brief Move to next field.
        /// \return Unchanged instance of iterator.
        /// \invariant Inside valid bounds, but not in the ending.
        PointRepresentationIterator operator ++ (int) noexcept;

        /// \brief Move to previous field.
        /// \invariant Inside valid bounds, but not in the beginning.
        PointRepresentationIterator &operator -- () noexcept;

        /// \brief Move to previous field.
        /// \return Unchanged instance of iterator.
        /// \invariant Inside valid bounds, but not in the beginning.
        PointRepresentationIterator operator -- (int) noexcept;

        bool operator == (const PointRepresentationIterator &_other) const noexcept;

        bool operator != (const PointRepresentationIterator &_other) const noexcept;

    private:
        /// Collection constructs iterators for point representations.
        friend class Collection;

        static constexpr std::size_t DATA_MAX_SIZE = sizeof (uintptr_t);

        explicit PointRepresentationIterator (const std::array <uint8_t, DATA_MAX_SIZE> *_data) noexcept;

        /// \brief Iterator implementation-specific data.
        std::array <uint8_t, DATA_MAX_SIZE> data;
    };

    /// \brief Allows iteration over Collection volumetric representations.
    ///
    /// \warning Collection::CreateVolumetricRepresentation invalidates these iterators.
    class VolumetricRepresentationIterator final
    {
    public:
        ~VolumetricRepresentationIterator () noexcept;

        /// \return VolumetricRepresentation, to which iterator points.
        /// \invariant Inside valid bounds, but not in the ending.
        VolumetricRepresentation operator * () const noexcept;

        /// \brief Move to next field.
        /// \invariant Inside valid bounds, but not in the ending.
        VolumetricRepresentationIterator &operator ++ () noexcept;

        /// \brief Move to next field.
        /// \return Unchanged instance of iterator.
        /// \invariant Inside valid bounds, but not in the ending.
        VolumetricRepresentationIterator operator ++ (int) noexcept;

        /// \brief Move to previous field.
        /// \invariant Inside valid bounds, but not in the beginning.
        VolumetricRepresentationIterator &operator -- () noexcept;

        /// \brief Move to previous field.
        /// \return Unchanged instance of iterator.
        /// \invariant Inside valid bounds, but not in the beginning.
        VolumetricRepresentationIterator operator -- (int) noexcept;

        bool operator == (const VolumetricRepresentationIterator &_other) const noexcept;

        bool operator != (const VolumetricRepresentationIterator &_other) const noexcept;

    private:
        /// Collection constructs iterators for volumetric representations.
        friend class Collection;

        static constexpr std::size_t DATA_MAX_SIZE = sizeof (uintptr_t);

        explicit VolumetricRepresentationIterator (const std::array <uint8_t, DATA_MAX_SIZE> *_data) noexcept;

        /// \brief Iterator implementation-specific data.
        std::array <uint8_t, DATA_MAX_SIZE> data;
    };

    /// \brief Describes one of the VolumetricRepresentation dimensions, used for VolumetricRepresentation creation.
    struct DimensionDescription
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
    LinearRepresentation CreateLinearRepresentation (StandardLayout::FieldId _keyField) const noexcept;

    /// \brief Adds PointRepresentation to Collection, that uses given _keyFields as point position.
    /// \invariant There is no active allocation transactions in this collection and cursors in its representations.
    PointRepresentation CreatePointRepresentation (std::vector <StandardLayout::FieldId> _keyFields) const noexcept;

    /// \brief Adds VolumetricRepresentation to Collection, that uses given _dimensions.
    /// \invariant There is no active allocation transactions in this collection and cursors in its representations.
    VolumetricRepresentation CreateVolumetricRepresentation (std::vector <DimensionDescription> _dimensions) const noexcept;

    /// \return Iterator, that points to beginning of linear representations range.
    LinearRepresentationIterator LinearRepresentationBegin () noexcept;

    /// \return Iterator, that points to ending of linear representations range.
    LinearRepresentationIterator LinearRepresentationEnd () noexcept;

    /// \return Iterator, that points to beginning of point representations range.
    PointRepresentationIterator PointRepresentationBegin () noexcept;

    /// \return Iterator, that points to ending of point representations range.
    PointRepresentationIterator PointRepresentationEnd () noexcept;

    /// \return Iterator, that points to beginning of volumetric representations range.
    VolumetricRepresentationIterator VolumetricRepresentationBegin () noexcept;

    /// \return Iterator, that points to ending of volumetric representations range.
    VolumetricRepresentationIterator VolumetricRepresentationEnd () noexcept;

    Collection &operator = (const Collection &_other) = delete;

    Collection &operator = (Collection &&_other) noexcept;

private:
    /// \brief Implementation handle.
    void *handle;
};
} // namespace Emergence::RecordCollection

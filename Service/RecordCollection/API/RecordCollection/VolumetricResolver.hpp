#pragma once

#include <array>
#include <cstdint>

#include <StandardLayout/Field.hpp>

namespace Emergence::RecordCollection
{
/// \brief Represents records as axis aligned bounded shapes in given dimensions.
///
/// \details For example, volumetric resolver with 3 dimensions can be
///          implemented as octree with axis aligned bounding boxes.
///          Works as handle to real point resolver instance.
///          Prevents destruction unless there is only one reference to instance.
class VolumetricResolver final
{
public:
    /// \brief Allows user to read records, that match criteria, specified in
    ///        VolumetricResolver::ReadShapeIntersections or VolumetricResolver::ReadRayIntersections.
    class ReadCursor final
    {
    public:
        ReadCursor (const ReadCursor &_other);

        ReadCursor (ReadCursor &&_other);

        ~ReadCursor ();

        /// \return Pointer to current record or nullptr if there is no more records.
        const void *operator * () const noexcept;

        /// \brief Moves cursor to next record.
        /// \invariant Cursor should not point to ending.
        ReadCursor &operator ++ () noexcept;

    private:
        /// VolumetricResolver constructs its cursors.
        friend class VolumetricResolver;

        static constexpr std::size_t DATA_MAX_SIZE = sizeof (uintptr_t) * 2u;

        explicit ReadCursor (const std::array <uint8_t, DATA_MAX_SIZE> *_data) noexcept;

        /// \brief Iterator implementation-specific data.
        std::array <uint8_t, DATA_MAX_SIZE> data;
    };

    /// \brief Allows user to read, modify and delete records, that match criteria, specified in
    ///        VolumetricResolver::EditShapeIntersections or VolumetricResolver::EditRayIntersections.
    class EditCursor final
    {
    public:
        /// Edit cursors can not be copied, because not more than one edit
        /// cursor can exist inside one Collection at any moment of time.
        EditCursor (const EditCursor &_other) = delete;

        EditCursor (EditCursor &&_other);

        ~EditCursor ();

        /// \return Pointer to current record or nullptr if there is no more records.
        void *operator * () noexcept;

        /// \brief Deletes current record from collection and moves to next record.
        ///
        /// \invariant Cursor should not point to ending.
        ///
        /// \warning Record type is unknown during compile time, therefore appropriate
        ///          destructor should be called before record deletion.
        EditCursor &operator ~ () noexcept;

        /// \brief Checks current record for key values changes. Then moves cursor to next record.
        /// \invariant Cursor should not point to ending.
        EditCursor &operator ++ () noexcept;

    private:
        /// VolumetricResolver constructs its cursors.
        friend class VolumetricResolver;

        static constexpr std::size_t DATA_MAX_SIZE = sizeof (uintptr_t) * 2u;

        explicit EditCursor (const std::array <uint8_t, DATA_MAX_SIZE> *_data) noexcept;

        /// \brief Iterator implementation-specific data.
        std::array <uint8_t, DATA_MAX_SIZE> data;
    };

    /// \brief Allows iteration over VolumetricResolver dimensions.
    class DimensionIterator final
    {
    public:
        /// \brief Describes one of resolver dimensions.
        struct Dimension
        {
            /// \brief Pointer to minimum possible value of #minField.
            ///
            /// \details Values, that are less than this value will be processed as this value.
            const void *globalMin;

            /// \brief Field, that holds record minimum border value for this dimension.
            StandardLayout::Field minField;

            /// \brief Pointer to maximum possible value of #maxField.
            ///
            /// \details Values, that are greater than this value will be processed as this value.
            const void *globalMax;

            /// \brief Field, that holds record maximum border value for this dimension.
            StandardLayout::Field maxField;
        };

        ~DimensionIterator () noexcept;

        /// \return Description of dimension, to which iterator points.
        /// \invariant Inside valid bounds, but not in the ending.
        Dimension operator * () const noexcept;

        /// \brief Move to next field.
        /// \invariant Inside valid bounds, but not in the ending.
        DimensionIterator &operator ++ () noexcept;

        /// \brief Move to next field.
        /// \return Unchanged instance of iterator.
        /// \invariant Inside valid bounds, but not in the ending.
        DimensionIterator operator ++ (int) noexcept;

        /// \brief Move to previous field.
        /// \invariant Inside valid bounds, but not in the beginning.
        DimensionIterator &operator -- () noexcept;

        /// \brief Move to previous field.
        /// \return Unchanged instance of iterator.
        /// \invariant Inside valid bounds, but not in the beginning.
        DimensionIterator operator -- (int) noexcept;

        bool operator == (const DimensionIterator &_other) const noexcept;

        bool operator != (const DimensionIterator &_other) const noexcept;

    private:
        /// VolumetricResolver constructs dimension iterators.
        friend class VolumetricResolver;

        static constexpr std::size_t DATA_MAX_SIZE = sizeof (uintptr_t);

        explicit DimensionIterator (const std::array <uint8_t, DATA_MAX_SIZE> *_data) noexcept;

        std::array <uint8_t, DATA_MAX_SIZE> data;
    };

    /// \brief Defines shape by specifying min-max value pair for each dimension.
    ///
    /// \details Dimension count and types are unknown during compile time, therefore Shape is a pointer to
    ///          memory block, that holds min-max pair of values for each dimension in correct order. For example,
    ///          if it's needed to describe rectangle with width equal to 3, height equal to 2, center in
    ///          (x = 1, y = 3} point and dimensions are x = {float x0; float x1;} and y = {float y0; float y1;},
    ///          then shape memory block should be {-0.5f, 2.5f, 2.0f, 4.0f}.
    ///
    /// \warning Due to runtime-only nature of shapes, logically incorrect pointers can not be caught.
    /// \invariant Should not be `nullptr`.
    using Shape = const uint8_t *;

    /// \brief Defines ray by specifying origin-direction value pair for each dimension.
    ///
    /// \details Dimension count and types are unknown during compile time, therefore Ray is a pointer to
    ///          memory block, that holds origin-direction pair of values for each dimension in correct order.
    ///          For example, if it's needed to describe ray with origin in (x = 2, y = 3) point,
    ///          (dx = 0.8, dy = -0.6) direction and dimensions are x = {float x0; float x1;} and
    ///          y = {float y0; float y1;}, then ray memory block should be {2.0f, 0.8f, 3.0f, -0.6f}.
    ///
    /// \warning Due to runtime-only nature of rays, logically incorrect pointers can not be caught.
    /// \invariant Should not be `nullptr`.
    using Ray = const uint8_t *;

    VolumetricResolver (const VolumetricResolver &_other) noexcept;

    VolumetricResolver (VolumetricResolver &&_other) noexcept;

    ~VolumetricResolver () noexcept;

    /// \brief Finds records, which shape representations intersect with given shape, and allows user to read them.
    ///
    /// \details Complexity -- amortized O(F(D)*lgN), where D is count of dimensions, F is
    ///          implementation specific function and N is count of records in Collection.
    /// \invariant There is no active insertion transactions and edit cursors in Collection.
    ReadCursor ReadShapeIntersections (Shape _shape) noexcept;

    /// \brief Finds records, which shape representations intersect
    ///        with given shape, and allows user to edit and delete them.
    ///
    /// \details Complexity -- amortized O(F(D)*lgN), where D is count of dimensions, F is
    ///          implementation specific function and N is count of records in Collection.
    /// \invariant There is no active insertion transactions and read or edit cursors in Collection.
    EditCursor EditShapeIntersections (Shape _shape) noexcept;

    /// \brief Finds records, which shape representations intersect
    ///        with [0.0, _rayLength] part of given ray, and allows user to read them.
    ///
    /// \details Complexity -- amortized O(F(D)*lgN), where D is count of dimensions, F is
    ///          implementation specific function and N is count of records in Collection.
    /// \invariant There is no active insertion transactions and edit cursors in Collection.
    ReadCursor ReadRayIntersections (Ray _ray, float _rayLength) noexcept;

    /// \brief Finds records, which shape representations intersect
    ///        with [0.0, _rayLength] part of given ray, and allows user to edit and delete them.
    ///
    /// \details Complexity -- amortized O(F(D)*lgN), where D is count of dimensions, F is
    ///          implementation specific function and N is count of records in Collection.
    /// \invariant There is no active insertion transactions and edit cursors in Collection.
    EditCursor EditRayIntersections (Ray _ray, float _rayLength) noexcept;

    /// \return Iterator, that points to beginning of dimensions sequence.
    DimensionIterator DimensionBegin () const noexcept;

    /// \return Iterator, that points to ending of dimensions sequence.
    DimensionIterator DimensionEnd () const noexcept;

    /// \return Can this resolver be safely dropped?
    /// \details Resolver can be safely dropped if there is only one reference to it and there is no active cursors.
    bool CanBeDropped () const;

    /// \brief Deletes this volumetric resolver from Collection.
    /// \invariant ::CanBeDropped
    void Drop ();

    // TODO: Assignment operators?

private:
    /// Collection constructs resolvers.
    friend class Collection;

    explicit VolumetricResolver (void *_handle) noexcept;

    /// \brief Implementation handle.
    void *handle;
};
} // namespace Emergence::RecordCollection

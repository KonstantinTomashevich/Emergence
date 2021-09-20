#pragma once

#include <array>
#include <cstdint>

#include <API/Common/Cursor.hpp>
#include <API/Common/ImplementationBinding.hpp>
#include <API/Common/Iterator.hpp>

#include <StandardLayout/Field.hpp>

namespace Emergence::RecordCollection
{
/// \brief Represents records as axis aligned bounded shapes in given dimensions.
///
/// \details For example, volumetric representation with 3 dimensions can be
///          implemented as octree with axis aligned bounding boxes.
///          Works as handle to real point representation instance.
///          Prevents destruction unless there is only one reference to instance.
class VolumetricRepresentation final
{
public:
    /// \brief Allows user to read records, that match criteria, specified
    ///        in VolumetricRepresentation::ReadShapeIntersections.
    /// \details All ShapeIntersectionReadCursor operations are thread safe.
    class ShapeIntersectionReadCursor final
    {
    public:
        EMERGENCE_READ_CURSOR_OPERATIONS (ShapeIntersectionReadCursor);

    private:
        /// VolumetricRepresentation constructs its cursors.
        friend class VolumetricRepresentation;

        /// Cursor implementation could copy Shape inside to be more cache coherent and Shape could contain doubles,
        /// which are 8-byte long on all architectures. Therefore we use uint64_t as base size type.
        EMERGENCE_BIND_IMPLEMENTATION_INPLACE (sizeof (uint64_t) * 21u);

        explicit ShapeIntersectionReadCursor (std::array<uint8_t, DATA_MAX_SIZE> *_data) noexcept;
    };

    /// \brief Allows user to read, modify and delete records, that match criteria,
    ///        specified in VolumetricRepresentation::EditShapeIntersections.
    class ShapeIntersectionEditCursor final
    {
    public:
        EMERGENCE_EDIT_CURSOR_OPERATIONS (ShapeIntersectionEditCursor);

    private:
        /// VolumetricRepresentation constructs its cursors.
        friend class VolumetricRepresentation;

        /// About uint64_t: see comment in ShapeIntersectionReadCursor.
        EMERGENCE_BIND_IMPLEMENTATION_INPLACE (sizeof (uint64_t) * 21u);

        explicit ShapeIntersectionEditCursor (std::array<uint8_t, DATA_MAX_SIZE> *_data) noexcept;
    };

    /// \brief Allows user to read records, that match criteria, specified
    ///        in VolumetricRepresentation::ReadRayIntersections.
    /// \details All RayIntersectionReadCursor operations are thread safe.
    class RayIntersectionReadCursor final
    {
    public:
        EMERGENCE_READ_CURSOR_OPERATIONS (RayIntersectionReadCursor);

    private:
        /// VolumetricRepresentation constructs its cursors.
        friend class VolumetricRepresentation;

        /// About uint64_t: same as in comment in ShapeIntersectionReadCursor, but for Ray caching.
        EMERGENCE_BIND_IMPLEMENTATION_INPLACE (sizeof (uint64_t) * 19u);

        explicit RayIntersectionReadCursor (std::array<uint8_t, DATA_MAX_SIZE> *_data) noexcept;
    };

    /// \brief Allows user to read, modify and delete records, that match criteria,
    ///        specified in VolumetricRepresentation::EditRayIntersections.
    class RayIntersectionEditCursor final
    {
    public:
        EMERGENCE_EDIT_CURSOR_OPERATIONS (RayIntersectionEditCursor);

    private:
        /// VolumetricRepresentation constructs its cursors.
        friend class VolumetricRepresentation;

        /// About uint64_t: see comment in RayIntersectionReadCursor.
        EMERGENCE_BIND_IMPLEMENTATION_INPLACE (sizeof (uint64_t) * 19u);

        explicit RayIntersectionEditCursor (std::array<uint8_t, DATA_MAX_SIZE> *_data) noexcept;
    };

    /// \brief Allows iteration over VolumetricRepresentation dimensions.
    class DimensionIterator final
    {
    public:
        /// \brief Describes one of representation dimensions.
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

        EMERGENCE_BIDIRECTIONAL_ITERATOR_OPERATIONS (DimensionIterator, Dimension);

    private:
        /// VolumetricRepresentation constructs dimension iterators.
        friend class VolumetricRepresentation;

        EMERGENCE_BIND_IMPLEMENTATION_INPLACE (sizeof (uintptr_t));

        explicit DimensionIterator (const std::array<uint8_t, DATA_MAX_SIZE> *_data) noexcept;
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
    using Shape = const void *;

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
    using Ray = const void *;

    VolumetricRepresentation (const VolumetricRepresentation &_other) noexcept;

    VolumetricRepresentation (VolumetricRepresentation &&_other) noexcept;

    ~VolumetricRepresentation () noexcept;

    /// \brief Finds records, which shape representations intersect with given shape, and allows user to read them.
    ///
    /// \details Complexity -- amortized O(F(D)*lgN), where D is count of dimensions, F is
    ///          implementation specific function and N is count of records in Collection.
    /// \invariant There is no active allocation transactions and edit cursors in Collection.
    ShapeIntersectionReadCursor ReadShapeIntersections (Shape _shape) noexcept;

    /// \brief Finds records, which shape representations intersect
    ///        with given shape, and allows user to edit and delete them.
    ///
    /// \details Complexity -- amortized O(F(D)*lgN), where D is count of dimensions, F is
    ///          implementation specific function and N is count of records in Collection.
    /// \invariant There is no active allocation transactions and read or edit cursors in Collection.
    ShapeIntersectionEditCursor EditShapeIntersections (Shape _shape) noexcept;

    /// \brief Finds records, which shape representations intersect
    ///        with [0.0, _rayLength] part of given ray, and allows user to read them.
    ///
    /// \details Complexity -- amortized O(F(D)*lgN), where D is count of dimensions, F is
    ///          implementation specific function and N is count of records in Collection.
    /// \invariant There is no active allocation transactions and edit cursors in Collection.
    RayIntersectionReadCursor ReadRayIntersections (Ray _ray, float _rayLength) noexcept;

    /// \brief Finds records, which shape representations intersect
    ///        with [0.0, _rayLength] part of given ray, and allows user to edit and delete them.
    ///
    /// \details Complexity -- amortized O(F(D)*lgN), where D is count of dimensions, F is
    ///          implementation specific function and N is count of records in Collection.
    /// \invariant There is no active allocation transactions and edit cursors in Collection.
    RayIntersectionEditCursor EditRayIntersections (Ray _ray, float _rayLength) noexcept;

    /// \return Iterator, that points to beginning of dimensions sequence.
    DimensionIterator DimensionBegin () const noexcept;

    /// \return Iterator, that points to ending of dimensions sequence.
    DimensionIterator DimensionEnd () const noexcept;

    /// \seeCollection::GetRecordMapping
    const StandardLayout::Mapping &GetTypeMapping () const noexcept;

    /// \return Can this representation be safely dropped?
    /// \details Representation can be safely dropped if there is only one reference to it and there is no active
    ///          cursors.
    bool CanBeDropped () const noexcept;

    /// \brief Deletes this volumetric representation from Collection.
    /// \invariant ::CanBeDropped
    void Drop () noexcept;

    /// \return True if this and given instances are handles to the same representation.
    bool operator== (const VolumetricRepresentation &_other) const noexcept;

    VolumetricRepresentation &operator= (const VolumetricRepresentation &_other) noexcept;

    VolumetricRepresentation &operator= (VolumetricRepresentation &&_other) noexcept;

private:
    /// Collection constructs representations.
    friend class Collection;

    explicit VolumetricRepresentation (void *_handle) noexcept;

    EMERGENCE_BIND_IMPLEMENTATION_HANDLE ();
};
} // namespace Emergence::RecordCollection

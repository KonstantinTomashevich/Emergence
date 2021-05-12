#pragma once

#include <array>
#include <cstdint>

#include <StandardLayout/Field.hpp>

namespace Emergence::RecordCollection
{
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
        EditCursor (const EditCursor &_other) = delete;

        EditCursor (EditCursor &&_other);

        ~EditCursor ();

        /// \return Pointer to current record or nullptr if there is no more records.
        void *operator * () noexcept;

        /// \brief Deletes current record from collection and moves to next record.
        /// \invariant Cursor should not point to ending.
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

    class DimensionIterator final
    {
    public:
        struct BorderFields
        {
            StandardLayout::Field min;

            StandardLayout::Field max;
        };

        ~DimensionIterator () noexcept;

        BorderFields operator * () const noexcept;

        DimensionIterator &operator ++ () noexcept;

        DimensionIterator operator ++ (int) noexcept;

        DimensionIterator &operator -- () noexcept;

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

    // TODO: Think about direction normalization requirement.
    //       It seems standard for floating point rays, but can not be applied to integer rays.

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

    ReadCursor ReadShapeIntersections (Shape _shape) noexcept;

    EditCursor EditShapeIntersections (Shape _shape) noexcept;

    ReadCursor ReadRayIntersections (Ray _ray) noexcept;

    EditCursor EditRayIntersections (Ray _ray) noexcept;

    DimensionIterator DimensionBegin () const noexcept;

    DimensionIterator DimensionEnd () const noexcept;

    bool CanBeDropped () const;

    void Drop ();

private:
    /// Collection constructs resolvers.
    friend class Collection;

    explicit VolumetricResolver (void *_handle) noexcept;

    /// \brief Implementation handle.
    void *handle;
};
} // namespace Emergence::RecordCollection

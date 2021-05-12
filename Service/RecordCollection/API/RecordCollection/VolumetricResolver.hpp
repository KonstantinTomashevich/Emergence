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

    /// \copydoc PointResolver::KeyFieldIterator
    class KeyFieldIterator final
    {
    public:
        ~KeyFieldIterator () noexcept;

        StandardLayout::Field operator * () const noexcept;

        KeyFieldIterator &operator ++ () noexcept;

        KeyFieldIterator operator ++ (int) noexcept;

        KeyFieldIterator &operator -- () noexcept;

        KeyFieldIterator operator -- (int) noexcept;

        bool operator == (const KeyFieldIterator &_other) const noexcept;

        bool operator != (const KeyFieldIterator &_other) const noexcept;

    private:
        /// VolumetricResolver constructs iterators for key fields.
        friend class VolumetricResolver;

        static constexpr std::size_t DATA_MAX_SIZE = sizeof (uintptr_t);

        explicit KeyFieldIterator (const std::array <uint8_t, DATA_MAX_SIZE> *_data) noexcept;

        std::array <uint8_t, DATA_MAX_SIZE> data;
    };

    VolumetricResolver (const VolumetricResolver &_other) noexcept;

    VolumetricResolver (VolumetricResolver &&_other) noexcept;

    ~VolumetricResolver () noexcept;

    ReadCursor ReadShapeIntersections (const uint8_t *_bounds) noexcept;

    EditCursor EditShapeIntersections (const uint8_t *_bounds) noexcept;

    ReadCursor ReadRayIntersections (const uint8_t *_rayDefinition) noexcept;

    EditCursor EditRayIntersections (const uint8_t *_rayDefinition) noexcept;

    KeyFieldIterator KeyFieldBegin () const noexcept;

    KeyFieldIterator KeyFieldEnd () const noexcept;

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

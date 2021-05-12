#pragma once

#include <array>
#include <cstdint>

#include <StandardLayout/Field.hpp>

namespace Emergence::RecordCollection
{
class PointResolver final
{
public:
    /// \brief Allows user to read records, that match criteria, specified in PointResolver::ReadPoint.
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
        /// PointResolver constructs its cursors.
        friend class PointResolver;

        static constexpr std::size_t DATA_MAX_SIZE = sizeof (uintptr_t) * 2u;

        explicit ReadCursor (const std::array <uint8_t, DATA_MAX_SIZE> *_data) noexcept;

        /// \brief Implementation-specific data.
        std::array <uint8_t, DATA_MAX_SIZE> data;
    };

    /// \brief Allows user to read, modify and delete records, that
    ///        match criteria, specified in PointResolver::EditPoint.
    class EditCursor final
    {
    public:
        EditCursor (const EditCursor &_other) = delete;

        EditCursor (EditCursor &&_other);

        ~EditCursor ();

        // TODO: /reminder We can check for changes in indexed fields in MoveToNextRecord and ~WriteCursor.
        //                 Usually indexed fields are quite small and there is not a lot of such fields,
        //                 therefore it's okay to copy them and check for equality later.

        /// \return Pointer to current record or nullptr if there is no more records.
        void *operator * () noexcept;

        /// \brief Deletes current record from collection and moves to next record.
        /// \invariant Cursor should not point to ending.
        EditCursor &operator ~ ();

        /// \brief Checks current record for key values changes. Then moves cursor to next record.
        /// \invariant Cursor should not point to ending.
        EditCursor &operator ++ () noexcept;

    private:
        /// PointResolver constructs its cursors.
        friend class PointResolver;

        static constexpr std::size_t DATA_MAX_SIZE = sizeof (uintptr_t) * 2u;

        explicit EditCursor (const std::array <uint8_t, DATA_MAX_SIZE> *_data) noexcept;

        /// \brief Implementation-specific data.
        std::array <uint8_t, DATA_MAX_SIZE> data;
    };

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
        /// PointResolver constructs iterators for key fields.
        friend class PointResolver;

        static constexpr std::size_t DATA_MAX_SIZE = sizeof (uintptr_t);

        explicit KeyFieldIterator (const std::array <uint8_t, DATA_MAX_SIZE> *_data) noexcept;

        /// \brief Iterator implementation-specific data.
        std::array <uint8_t, DATA_MAX_SIZE> data;
    };

    /// \brief Defines point by specifying value for each key field.
    ///
    /// \details Key field count and types are unknown during compile time, therefore Point is
    ///          a pointer to memory block, that holds values for each key field in correct order.
    ///
    /// \warning Due to runtime-only nature of points, logically incorrect pointers can not be caught.
    /// \invariant Should not be `nullptr`.
    using Point = const uint8_t *;

    PointResolver (const PointResolver &_other) noexcept;

    PointResolver (PointResolver &&_other) noexcept;

    ~PointResolver () noexcept;

    ReadCursor ReadPoint (Point _point) noexcept;

    EditCursor EditPoint (Point _point) noexcept;

    KeyFieldIterator KeyFieldBegin () const noexcept;

    KeyFieldIterator KeyFieldEnd () const noexcept;

    bool CanBeDropped () const;

    void Drop ();

private:
    /// Collection constructs resolvers.
    friend class Collection;

    explicit PointResolver (void *_handle) noexcept;

    /// \brief Implementation handle.
    void *handle;
};
} // namespace Emergence::RecordCollection

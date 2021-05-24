#pragma once

#include <array>
#include <cstdint>

#include <StandardLayout/Field.hpp>

namespace Emergence::RecordCollection
{
/// \brief Projects records to set of points, where point is a vector of values from given fields.
///
/// \details Works as handle to real point resolver instance.
///          Prevents destruction unless there is only one reference to instance.
class PointResolver final
{
public:
    /// \brief Allows user to read records, that match criteria, specified in PointResolver::ReadPoint.
    class ReadCursor final
    {
    public:
        ReadCursor (const ReadCursor &_other) noexcept;

        ReadCursor (ReadCursor &&_other) noexcept;

        ~ReadCursor () noexcept;

        /// \return Pointer to current record or nullptr if there is no more records.
        const void *operator * () const noexcept;

        /// \brief Moves cursor to next record.
        /// \invariant Cursor should not point to ending.
        ReadCursor &operator ++ () noexcept;

        ReadCursor &operator = (const ReadCursor &_other) noexcept;

        ReadCursor &operator = (ReadCursor &&_other) noexcept;

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
        /// Edit cursors can not be copied, because not more than one edit
        /// cursor can exist inside one Collection at any moment of time.
        EditCursor (const EditCursor &_other) = delete;

        EditCursor (EditCursor &&_other) noexcept;

        ~EditCursor () noexcept;

        // TODO: /reminder We can check for changes in indexed fields in MoveToNextRecord and ~WriteCursor.
        //                 Usually indexed fields are quite small and there is not a lot of such fields,
        //                 therefore it's okay to copy them and check for equality later.

        /// \return Pointer to current record or nullptr if there is no more records.
        void *operator * () noexcept;

        /// \brief Deletes current record from collection and moves to next record.
        ///
        /// \invariant Cursor should not point to ending.
        ///
        /// \warning Record type is unknown during compile time, therefore appropriate
        ///          destructor should be called before record deletion.
        EditCursor &operator ~ ();

        /// \brief Checks current record for key values changes. Then moves cursor to next record.
        /// \invariant Cursor should not point to ending.
        EditCursor &operator ++ () noexcept;

        EditCursor &operator = (const EditCursor &_other) = delete;

        EditCursor &operator = (EditCursor &&_other) noexcept;

    private:
        /// PointResolver constructs its cursors.
        friend class PointResolver;

        static constexpr std::size_t DATA_MAX_SIZE = sizeof (uintptr_t) * 2u;

        explicit EditCursor (const std::array <uint8_t, DATA_MAX_SIZE> *_data) noexcept;

        /// \brief Implementation-specific data.
        std::array <uint8_t, DATA_MAX_SIZE> data;
    };

    /// \brief Allows iteration over PointResolver key fields.
    class KeyFieldIterator final
    {
    public:
        ~KeyFieldIterator () noexcept;

        /// \return Key Field, to which iterator points.
        /// \invariant Inside valid bounds, but not in the ending.
        StandardLayout::Field operator * () const noexcept;

        /// \brief Move to next field.
        /// \invariant Inside valid bounds, but not in the ending.
        KeyFieldIterator &operator ++ () noexcept;

        /// \brief Move to next field.
        /// \return Unchanged instance of iterator.
        /// \invariant Inside valid bounds, but not in the ending.
        KeyFieldIterator operator ++ (int) noexcept;

        /// \brief Move to previous field.
        /// \invariant Inside valid bounds, but not in the beginning.
        KeyFieldIterator &operator -- () noexcept;

        /// \brief Move to previous field.
        /// \return Unchanged instance of iterator.
        /// \invariant Inside valid bounds, but not in the beginning.
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

    /// \brief Finds point, described by given values, and allows user to read records from it.
    ///
    /// \details Complexity -- O(C*N), where C is amortized constant and N is total size of key fields in bytes.
    /// \invariant There is no active insertion transactions and edit cursors in Collection.
    ReadCursor ReadPoint (Point _point) noexcept;

    /// \brief Finds point, described by given values, and allows user to edit and delete records from this point.
    ///
    /// \details Complexity -- O(C*N), where C is amortized constant and N is total size of key fields in bytes.
    /// \invariant There is no active insertion transactions and read or edit cursors in Collection.
    EditCursor EditPoint (Point _point) noexcept;

    /// \return Iterator, that points to beginning of key fields sequence.
    KeyFieldIterator KeyFieldBegin () const noexcept;

    /// \return Iterator, that points to ending of key fields sequence.
    KeyFieldIterator KeyFieldEnd () const noexcept;

    /// \return Can this resolver be safely dropped?
    /// \details Resolver can be safely dropped if there is only one reference to it and there is no active cursors.
    bool CanBeDropped () const noexcept;

    /// \brief Deletes this point resolver from Collection.
    /// \invariant ::CanBeDropped
    void Drop () noexcept;

    PointResolver &operator = (const PointResolver &_other) noexcept;

    PointResolver &operator = (PointResolver &&_other) noexcept;

private:
    /// Collection constructs resolvers.
    friend class Collection;

    explicit PointResolver (void *_handle) noexcept;

    /// \brief Implementation handle.
    void *handle;
};
} // namespace Emergence::RecordCollection

#pragma once

#include <array>
#include <cstdint>

#include <StandardLayout/Field.hpp>

namespace Emergence::RecordCollection
{
/// \brief Projects records to set of points, where point is a vector of values from given fields.
///
/// \details Works as handle to real point representation instance.
///          Prevents destruction unless there is only one reference to instance.
class PointRepresentation final
{
public:
    /// \brief Allows user to read records, that match criteria, specified in PointRepresentation::ReadPoint.
    /// \details All ReadCursor operations are thread safe.
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

        /// Assigning cursors looks counter intuitive.
        ReadCursor &operator = (const ReadCursor &_other) = delete;

        /// Assigning cursors looks counter intuitive.
        ReadCursor &operator = (ReadCursor &&_other) = delete;

    private:
        /// PointRepresentation constructs its cursors.
        friend class PointRepresentation;

        static constexpr std::size_t DATA_MAX_SIZE = sizeof (uintptr_t) * 2u;

        explicit ReadCursor (const std::array <uint8_t, DATA_MAX_SIZE> *_data) noexcept;

        /// \brief Implementation-specific data.
        std::array <uint8_t, DATA_MAX_SIZE> data;
    };

    /// \brief Allows user to read, modify and delete records, that
    ///        match criteria, specified in PointRepresentation::EditPoint.
    class EditCursor final
    {
    public:
        /// Edit cursors can not be copied, because not more than one edit
        /// cursor can exist inside one Collection at any moment of time.
        EditCursor (const EditCursor &_other) = delete;

        EditCursor (EditCursor &&_other) noexcept;

        ~EditCursor () noexcept;

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

        /// Assigning cursors looks counter intuitive.
        EditCursor &operator = (const EditCursor &_other) = delete;

        /// Assigning cursors looks counter intuitive.
        EditCursor &operator = (EditCursor &&_other) = delete;

    private:
        /// PointRepresentation constructs its cursors.
        friend class PointRepresentation;

        static constexpr std::size_t DATA_MAX_SIZE = sizeof (uintptr_t) * 2u;

        explicit EditCursor (const std::array <uint8_t, DATA_MAX_SIZE> *_data) noexcept;

        /// \brief Implementation-specific data.
        std::array <uint8_t, DATA_MAX_SIZE> data;
    };

    /// \brief Allows iteration over PointRepresentation key fields.
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
        /// PointRepresentation constructs iterators for key fields.
        friend class PointRepresentation;

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
    /// \invariant Values must be stored one after another without paddings in the same order as key fields.
    /// \invariant Value size for fields with StandardLayout::FieldArchetype::STRING must always be equal to
    ///            StandardLayout::Field::GetSize, even if string length is less than this value.
    /// \invariant Values for fields with StandardLayout::FieldArchetype::BIT must passed as bytes in which all
    ///            bits should be zero's except bit with StandardLayout::Field::GetBitOffset.
    using Point = const uint8_t *;

    /// There is no sense to copy representations, because they are part of Collection.
    PointRepresentation (const PointRepresentation &_other) = delete;

    /// Moving representations is forbidden, because otherwise user can move representation out of Collection.
    PointRepresentation (PointRepresentation &&_other) = delete;

    ~PointRepresentation () noexcept;

    /// \brief Finds point, described by given values, and allows user to read records from it.
    ///
    /// \details Complexity -- O(C*N), where C is amortized constant and N is total size of key fields in bytes.
    /// \invariant There is no active allocation transactions and edit cursors in Collection.
    ReadCursor ReadPoint (Point _point) noexcept;

    /// \brief Finds point, described by given values, and allows user to edit and delete records from this point.
    ///
    /// \details Complexity -- O(C*N), where C is amortized constant and N is total size of key fields in bytes.
    /// \invariant There is no active allocation transactions and read or edit cursors in Collection.
    EditCursor EditPoint (Point _point) noexcept;

    /// \return Iterator, that points to beginning of key fields sequence.
    KeyFieldIterator KeyFieldBegin () const noexcept;

    /// \return Iterator, that points to ending of key fields sequence.
    KeyFieldIterator KeyFieldEnd () const noexcept;

    /// \return Can this representation be safely dropped?
    /// \details Representation can be safely dropped if there is only one reference to it and there is no active cursors.
    bool CanBeDropped () const noexcept;

    /// \brief Deletes this point representation from Collection.
    /// \invariant ::CanBeDropped
    void Drop () noexcept;

    /// There is no sense to copy assign representations, because they are part of Collection.
    PointRepresentation &operator = (const PointRepresentation &_other) = delete;

    /// Move assigning representations is forbidden, because otherwise user can move representation out of Collection.
    PointRepresentation &operator = (PointRepresentation &&_other) = delete;

private:
    /// Collection constructs representations.
    friend class Collection;

    explicit PointRepresentation (void *_handle) noexcept;

    /// \brief Implementation handle.
    void *handle;
};
} // namespace Emergence::RecordCollection

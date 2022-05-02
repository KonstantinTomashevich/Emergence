#pragma once

#include <array>
#include <cstdint>

#include <API/Common/Cursor.hpp>
#include <API/Common/ImplementationBinding.hpp>
#include <API/Common/Iterator.hpp>

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
        EMERGENCE_READ_CURSOR_OPERATIONS (ReadCursor);

    private:
        /// PointRepresentation constructs its cursors.
        friend class PointRepresentation;

        EMERGENCE_BIND_IMPLEMENTATION_INPLACE (sizeof (uintptr_t) * 3u);

        explicit ReadCursor (std::array<uint8_t, DATA_MAX_SIZE> &_data) noexcept;
    };

    /// \brief Allows user to read, modify and delete records, that
    ///        match criteria, specified in PointRepresentation::EditPoint.
    class EditCursor final
    {
    public:
        EMERGENCE_EDIT_CURSOR_OPERATIONS (EditCursor);

    private:
        /// PointRepresentation constructs its cursors.
        friend class PointRepresentation;

        EMERGENCE_BIND_IMPLEMENTATION_INPLACE (sizeof (uintptr_t) * 3u);

        explicit EditCursor (std::array<uint8_t, DATA_MAX_SIZE> &_data) noexcept;
    };

    /// \brief Allows iteration over PointRepresentation key fields.
    class KeyFieldIterator final
    {
    public:
        EMERGENCE_BIDIRECTIONAL_ITERATOR_OPERATIONS (KeyFieldIterator, StandardLayout::Field);

    private:
        /// PointRepresentation constructs iterators for key fields.
        friend class PointRepresentation;

        EMERGENCE_BIND_IMPLEMENTATION_INPLACE (sizeof (uintptr_t));

        explicit KeyFieldIterator (const std::array<uint8_t, DATA_MAX_SIZE> &_data) noexcept;
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
    using Point = const void *;

    PointRepresentation (const PointRepresentation &_other) noexcept;

    PointRepresentation (PointRepresentation &&_other) noexcept;

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
    [[nodiscard]] KeyFieldIterator KeyFieldBegin () const noexcept;

    /// \return Iterator, that points to ending of key fields sequence.
    [[nodiscard]] KeyFieldIterator KeyFieldEnd () const noexcept;

    /// \see Collection::GetRecordMapping
    [[nodiscard]] const StandardLayout::Mapping &GetTypeMapping () const noexcept;

    /// \return Can this representation be safely dropped?
    /// \details Representation can be safely dropped if there is only one reference to it and there is no active
    ///          cursors.
    [[nodiscard]] bool CanBeDropped () const noexcept;

    /// \brief Deletes this point representation from Collection.
    /// \invariant ::CanBeDropped
    void Drop () noexcept;

    /// \return True if this and given instances are handles to the same representation.
    bool operator== (const PointRepresentation &_other) const noexcept;

    PointRepresentation &operator= (const PointRepresentation &_other) noexcept;

    PointRepresentation &operator= (PointRepresentation &&_other) noexcept;

private:
    /// Collection constructs representations.
    friend class Collection;

    explicit PointRepresentation (void *_handle) noexcept;

    EMERGENCE_BIND_IMPLEMENTATION_HANDLE ();
};
} // namespace Emergence::RecordCollection

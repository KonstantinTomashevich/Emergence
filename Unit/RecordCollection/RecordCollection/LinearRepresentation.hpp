#pragma once

#include <array>
#include <cstdint>

#include <API/Common/Cursor.hpp>
#include <API/Common/ImplementationBinding.hpp>

#include <StandardLayout/Field.hpp>

namespace Emergence::RecordCollection
{
/// \brief Represents records as sequence, in which records are sorted by value of given field in ascending order.
///
/// \details Works as handle to real linear representation instance.
///          Prevents destruction unless there is only one reference to instance.
class LinearRepresentation final
{
public:
    /// \brief Allows user to read records, that match criteria,
    ///        specified in LinearRepresentation::ReadAscendingInterval.
    /// \details All AscendingReadCursor operations are thread safe.
    class AscendingReadCursor final
    {
    public:
        EMERGENCE_READ_CURSOR_OPERATIONS (AscendingReadCursor);

    private:
        /// LinearRepresentation constructs its cursors.
        friend class LinearRepresentation;

        EMERGENCE_BIND_IMPLEMENTATION_INPLACE (sizeof (std::uintptr_t) * 3u);

        explicit AscendingReadCursor (std::array<std::uint8_t, DATA_MAX_SIZE> &_data) noexcept;
    };

    /// \brief Allows user to read, modify and delete records, that
    ///        match criteria, specified in LinearRepresentation::EditAscendingInterval.
    class AscendingEditCursor final
    {
    public:
        EMERGENCE_EDIT_CURSOR_OPERATIONS (AscendingEditCursor);

    private:
        /// LinearRepresentation constructs its cursors.
        friend class LinearRepresentation;

        EMERGENCE_BIND_IMPLEMENTATION_INPLACE (sizeof (std::uintptr_t) * 3u);

        explicit AscendingEditCursor (std::array<std::uint8_t, DATA_MAX_SIZE> &_data) noexcept;
    };

    /// \brief Allows user to read records, that match criteria,
    ///        specified in LinearRepresentation::ReadDescendingInterval.
    /// \details All DescendingReadCursor operations are thread safe.
    class DescendingReadCursor final
    {
    public:
        EMERGENCE_READ_CURSOR_OPERATIONS (DescendingReadCursor);

    private:
        /// LinearRepresentation constructs its cursors.
        friend class LinearRepresentation;

        EMERGENCE_BIND_IMPLEMENTATION_INPLACE (sizeof (std::uintptr_t) * 3u);

        explicit DescendingReadCursor (std::array<std::uint8_t, DATA_MAX_SIZE> &_data) noexcept;
    };

    /// \brief Allows user to read, modify and delete records, that
    ///        match criteria, specified in LinearRepresentation::EditDescendingInterval.
    class DescendingEditCursor final
    {
    public:
        EMERGENCE_EDIT_CURSOR_OPERATIONS (DescendingEditCursor);

    private:
        /// LinearRepresentation constructs its cursors.
        friend class LinearRepresentation;

        EMERGENCE_BIND_IMPLEMENTATION_INPLACE (sizeof (std::uintptr_t) * 3u);

        explicit DescendingEditCursor (std::array<std::uint8_t, DATA_MAX_SIZE> &_data) noexcept;
    };

    /// \brief Points to value, that defines one of interval borders.
    ///
    /// \details Key field type is unknown during compile time, therefore value is a pointer to memory
    ///          block with actual value. `nullptr` values will be interpreted as absence of borders.
    ///
    /// \warning Due to runtime-only nature of values, logically incorrect pointers can not be caught.
    using KeyFieldValue = const void *;

    LinearRepresentation (const LinearRepresentation &_other) noexcept;

    LinearRepresentation (LinearRepresentation &&_other) noexcept;

    ~LinearRepresentation () noexcept;

    /// \brief Finds interval, described by given borders, and allows user to read records from it.
    /// \details Nullptr border pointer will be interpreted as `no border`.
    ///          Cursor moves from lowest record to greatest in requested interval.
    ///
    /// \details Complexity -- O(lgN), where N is count of records in Collection.
    /// \invariant There is no active allocation transactions and edit cursors in Collection.
    AscendingReadCursor ReadAscendingInterval (KeyFieldValue _min, KeyFieldValue _max) noexcept;

    /// \brief Finds interval, described by given borders,
    ///        and allows user to edit and delete records from this interval.
    /// \details Nullptr border pointer will be interpreted as `no border`.
    ///          Cursor moves from lowest record to greatest in requested interval.
    ///
    /// \details Complexity -- O(lgN), where N is count of records in Collection.
    /// \invariant There is no active allocation transactions and read or edit cursors in Collection.
    AscendingEditCursor EditAscendingInterval (KeyFieldValue _min, KeyFieldValue _max) noexcept;

    /// \brief Same as ::ReadAscendingInterval, but records will be returned in descending order instead.
    DescendingReadCursor ReadDescendingInterval (KeyFieldValue _min, KeyFieldValue _max) noexcept;

    /// \brief Same as ::EditAscendingInterval, but records will be returned in descending order instead.
    DescendingEditCursor EditDescendingInterval (KeyFieldValue _min, KeyFieldValue _max) noexcept;

    /// \return Field, by which records are sorted in this linear representation.
    [[nodiscard]] StandardLayout::Field GetKeyField () const noexcept;

    /// \see Collection::GetRecordMapping
    [[nodiscard]] const StandardLayout::Mapping &GetTypeMapping () const noexcept;

    /// \return Can this representation be safely dropped?
    /// \details Representation can be safely dropped if there is only one reference to it and there is no active
    ///          cursors.
    [[nodiscard]] bool CanBeDropped () const noexcept;

    /// \brief Deletes this linear representation from Collection.
    /// \invariant ::CanBeDropped
    void Drop () noexcept;

    /// \return True if this and given instances are handles to the same representation.
    bool operator== (const LinearRepresentation &_other) const noexcept;

    LinearRepresentation &operator= (const LinearRepresentation &_other) noexcept;

    LinearRepresentation &operator= (LinearRepresentation &&_other) noexcept;

private:
    /// Collection constructs representations.
    friend class Collection;

    explicit LinearRepresentation (void *_handle) noexcept;

    EMERGENCE_BIND_IMPLEMENTATION_HANDLE ()
};
} // namespace Emergence::RecordCollection

#pragma once

#include <array>
#include <cstdint>

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
    /// \brief Allows user to read records, that match criteria, specified in LinearRepresentation::ReadInterval.
    /// \details All ReadCursor operations are thread safe.
    class ReadCursor final
    {
    public:
        ReadCursor (const ReadCursor &_other) noexcept;

        ReadCursor (ReadCursor &&_other) noexcept;

        ~ReadCursor () noexcept;

        /// \return Pointer to current record or nullptr if cursor points to ending.
        const void *operator * () const noexcept;

        /// \brief Moves cursor to next record.
        /// \invariant Cursor should not point to interval ending.
        ReadCursor &operator ++ () noexcept;

        /// Assigning cursors looks counter intuitive.
        ReadCursor &operator = (const ReadCursor &_other) = delete;

        /// Assigning cursors looks counter intuitive.
        ReadCursor &operator = (ReadCursor &&_other) = delete;

    private:
        /// LinearRepresentation constructs its cursors.
        friend class LinearRepresentation;

        static constexpr std::size_t DATA_MAX_SIZE = sizeof (uintptr_t) * 3u;

        explicit ReadCursor (std::array <uint8_t, DATA_MAX_SIZE> *_data) noexcept;

        /// \brief Iterator implementation-specific data.
        std::array <uint8_t, DATA_MAX_SIZE> data;
    };

    /// \brief Allows user to read, modify and delete records, that
    ///        match criteria, specified in LinearRepresentation::EditInterval.
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
        EditCursor &operator ~ () noexcept;

        /// \brief Checks current record for key values changes. Then moves cursor to next record.
        /// \invariant Cursor should not point to ending.
        EditCursor &operator ++ () noexcept;

        /// Assigning cursors looks counter intuitive.
        EditCursor &operator = (const EditCursor &_other) = delete;

        /// Assigning cursors looks counter intuitive.
        EditCursor &operator = (EditCursor &&_other) = delete;

    private:
        /// LinearRepresentation constructs its cursors.
        friend class LinearRepresentation;

        static constexpr std::size_t DATA_MAX_SIZE = sizeof (uintptr_t) * 3u;

        explicit EditCursor (std::array <uint8_t, DATA_MAX_SIZE> *_data) noexcept;

        /// \brief Iterator implementation-specific data.
        std::array <uint8_t, DATA_MAX_SIZE> data;
    };

    /// \brief Allows user to read records, that match criteria, specified in
    ///        LinearRepresentation::ReadReversedInterval.
    /// \details All ReversedReadCursor operations are thread safe.
    class ReversedReadCursor final
    {
    public:
        ReversedReadCursor (const ReversedReadCursor &_other) noexcept;

        ReversedReadCursor (ReversedReadCursor &&_other) noexcept;

        ~ReversedReadCursor () noexcept;

        /// \return Pointer to current record or nullptr if cursor points to ending.
        const void *operator * () const noexcept;

        /// \brief Moves cursor to next record.
        /// \invariant Cursor should not point to interval ending.
        ReversedReadCursor &operator ++ () noexcept;

        /// Assigning cursors looks counter intuitive.
        ReversedReadCursor &operator = (const ReversedReadCursor &_other) = delete;

        /// Assigning cursors looks counter intuitive.
        ReversedReadCursor &operator = (ReversedReadCursor &&_other) = delete;

    private:
        /// LinearRepresentation constructs its cursors.
        friend class LinearRepresentation;

        static constexpr std::size_t DATA_MAX_SIZE = sizeof (uintptr_t) * 3u;

        explicit ReversedReadCursor (std::array <uint8_t, DATA_MAX_SIZE> *_data) noexcept;

        /// \brief Iterator implementation-specific data.
        std::array <uint8_t, DATA_MAX_SIZE> data;
    };

    /// \brief Allows user to read, modify and delete records, that
    ///        match criteria, specified in LinearRepresentation::EditReversedInterval.
    class ReversedEditCursor final
    {
    public:
        /// Edit cursors can not be copied, because not more than one edit
        /// cursor can exist inside one Collection at any moment of time.
        ReversedEditCursor (const ReversedEditCursor &_other) = delete;

        ReversedEditCursor (ReversedEditCursor &&_other) noexcept;

        ~ReversedEditCursor () noexcept;

        /// \return Pointer to current record or nullptr if there is no more records.
        void *operator * () noexcept;

        /// \brief Deletes current record from collection and moves to next record.
        ///
        /// \invariant Cursor should not point to ending.
        ///
        /// \warning Record type is unknown during compile time, therefore appropriate
        ///          destructor should be called before record deletion.
        ReversedEditCursor &operator ~ () noexcept;

        /// \brief Checks current record for key values changes. Then moves cursor to next record.
        /// \invariant Cursor should not point to ending.
        ReversedEditCursor &operator ++ () noexcept;

        /// Assigning cursors looks counter intuitive.
        ReversedEditCursor &operator = (const ReversedEditCursor &_other) = delete;

        /// Assigning cursors looks counter intuitive.
        ReversedEditCursor &operator = (ReversedEditCursor &&_other) = delete;

    private:
        /// LinearRepresentation constructs its cursors.
        friend class LinearRepresentation;

        static constexpr std::size_t DATA_MAX_SIZE = sizeof (uintptr_t) * 3u;

        explicit ReversedEditCursor (std::array <uint8_t, DATA_MAX_SIZE> *_data) noexcept;

        /// \brief Iterator implementation-specific data.
        std::array <uint8_t, DATA_MAX_SIZE> data;
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
    ///
    /// \details Complexity -- O(lgN), where N is count of records in Collection.
    /// \invariant There is no active allocation transactions and edit cursors in Collection.
    ReadCursor ReadInterval (KeyFieldValue _min, KeyFieldValue _max) noexcept;

    /// \brief Finds interval, described by given borders,
    ///        and allows user to edit and delete records from this interval.
    /// \details Nullptr border pointer will be interpreted as `no border`.
    ///
    /// \details Complexity -- O(lgN), where N is count of records in Collection.
    /// \invariant There is no active allocation transactions and read or edit cursors in Collection.
    EditCursor EditInterval (KeyFieldValue _min, KeyFieldValue _max) noexcept;

    /// \brief Same as ::ReadInterval, but returned cursor will move from last record in interval to first.
    ReversedReadCursor ReadReversedInterval (KeyFieldValue _min, KeyFieldValue _max) noexcept;

    /// \brief Same as ::EditInterval, but returned cursor will move from last record in interval to first.
    ReversedEditCursor EditReversedInterval (KeyFieldValue _min, KeyFieldValue _max) noexcept;

    /// \return Field, by which records are sorted in this linear representation.
    StandardLayout::Field GetKeyField () const noexcept;

    // TODO: What about automatic drop feature? It seams quite useful for complex layered systems like Warehouse.

    /// \return Can this representation be safely dropped?
    /// \details Representation can be safely dropped if there is only one reference to it and there is no active cursors.
    bool CanBeDropped () const noexcept;

    /// \brief Deletes this linear representation from Collection.
    /// \invariant ::CanBeDropped
    void Drop () noexcept;

    /// \return True if this and given instances are handles to the same representation.
    bool operator == (const LinearRepresentation &_other) const noexcept;

    LinearRepresentation &operator = (const LinearRepresentation &_other) noexcept;

    LinearRepresentation &operator = (LinearRepresentation &&_other) noexcept;

private:
    /// Collection constructs representations.
    friend class Collection;

    explicit LinearRepresentation (void *_handle) noexcept;

    /// \brief Implementation handle.
    void *handle;
};
} // namespace Emergence::RecordCollection

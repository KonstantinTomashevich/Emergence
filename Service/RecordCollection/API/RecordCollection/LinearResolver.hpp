#pragma once

#include <array>
#include <cstdint>

#include <StandardLayout/Field.hpp>

namespace Emergence::RecordCollection
{
/// \brief Represents records as sequence, in which records are sorted by value of given field in ascending order.
///
/// \details Works as handle to real linear resolver instance.
///          Prevents destruction unless there is only one reference to instance.
class LinearResolver final
{
public:
    /// \brief Allows user to read records, that match criteria, specified in LinearResolver::ReadInterval.
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

        /// \brief Moves cursor to next record.
        /// \invariant Cursor should not point to interval beginning.
        ReadCursor &operator -- () noexcept;

        /// \brief Moves cursor to interval beginning.
        void MoveToBeginning () noexcept;

        /// \brief Moves cursor to interval ending.
        void MoveToEnding () noexcept;

        /// \return Does cursor point to interval beginning?
        bool IsInBeginning () const noexcept;

        /// Assigning cursors looks counter intuitive.
        ReadCursor &operator = (const ReadCursor &_other) = delete;

        /// Assigning cursors looks counter intuitive.
        ReadCursor &operator = (ReadCursor &&_other) = delete;

    private:
        /// LinearResolver constructs its cursors.
        friend class LinearResolver;

        static constexpr std::size_t DATA_MAX_SIZE = sizeof (uintptr_t) * 2u;

        explicit ReadCursor (const std::array <uint8_t, DATA_MAX_SIZE> *_data) noexcept;

        /// \brief Iterator implementation-specific data.
        std::array <uint8_t, DATA_MAX_SIZE> data;
    };

    /// \brief Allows user to read, modify and delete records, that
    ///        match criteria, specified in LinearResolver::EditInterval.
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

        /// \brief Moves cursor to next record.
        /// \invariant Cursor should not point to interval beginning.
        ReadCursor &operator -- () noexcept;

        /// \brief Moves cursor to interval beginning.
        void MoveToBeginning () noexcept;

        /// \brief Moves cursor to interval ending.
        void MoveToEnding () noexcept;

        /// \return Does cursor point to interval beginning?
        bool IsInBeginning () const noexcept;

        /// Assigning cursors looks counter intuitive.
        EditCursor &operator = (const EditCursor &_other) = delete;

        /// Assigning cursors looks counter intuitive.
        EditCursor &operator = (EditCursor &&_other) = delete;

    private:
        /// LinearResolver constructs its cursors.
        friend class LinearResolver;

        static constexpr std::size_t DATA_MAX_SIZE = sizeof (uintptr_t) * 2u;

        explicit EditCursor (const std::array <uint8_t, DATA_MAX_SIZE> *_data) noexcept;

        /// \brief Iterator implementation-specific data.
        std::array <uint8_t, DATA_MAX_SIZE> data;
    };

    /// \brief Points to value, that defines one of interval borders.
    ///
    /// \details Key field type is unknown during compile time, therefore value is a pointer to memory
    ///          block with actual value. `nullptr` values will be interpreted as absence of borders.
    ///
    /// \warning Due to runtime-only nature of values, logically incorrect pointers can not be caught.
    using KeyFieldValue = const uint8_t *;

    LinearResolver (const LinearResolver &_other) noexcept;

    LinearResolver (LinearResolver &&_other) noexcept;

    ~LinearResolver () noexcept;

    /// \brief Finds interval, described by given borders, and allows user to read records from it.
    ///
    /// \details Complexity -- O(lgN), where N is count of records in Collection.
    /// \invariant There is no active allocation transactions and edit cursors in Collection.
    ReadCursor ReadInterval (KeyFieldValue _min, KeyFieldValue _max) noexcept;

    /// \brief Finds interval, described by given borders,
    ///        and allows user to edit and delete records from this interval.
    ///
    /// \details Complexity -- O(lgN), where N is count of records in Collection.
    /// \invariant There is no active allocation transactions and read or edit cursors in Collection.
    EditCursor EditInterval (KeyFieldValue _min, KeyFieldValue _max) noexcept;

    /// \return Field, by which records are sorted in this linear resolver.
    StandardLayout::Field GetKeyField () const noexcept;

    /// \return Can this resolver be safely dropped?
    /// \details Resolver can be safely dropped if there is only one reference to it and there is no active cursors.
    bool CanBeDropped () const noexcept;

    /// \brief Deletes this linear resolver from Collection.
    /// \invariant ::CanBeDropped
    void Drop () noexcept;

    LinearResolver &operator = (const LinearResolver &_other) noexcept;

    LinearResolver &operator = (LinearResolver &&_other) noexcept;

private:
    /// Collection constructs resolvers.
    friend class Collection;

    explicit LinearResolver (void *_handle) noexcept;

    /// \brief Implementation handle.
    void *handle;
};
} // namespace Emergence::RecordCollection

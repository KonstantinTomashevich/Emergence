#pragma once

#include <RecordCollectionApi.hpp>

#include <array>
#include <cstdint>

#include <API/Common/Cursor.hpp>
#include <API/Common/ImplementationBinding.hpp>

#include <StandardLayout/Field.hpp>

namespace Emergence::RecordCollection
{
/// \brief Contains only records, that have given "signaled" value in given field.
///
/// \details This representation is a special case of PointRepresentation: we can think of it as a representation of
///          one specific point, given during construction, in one-dimensional area. SignalRepresentation is intended
///          to be used instead of PointRepresentation when user is interested only in one such point. For example:
///
///          - User needs to retrieve all records with `true` value of boolean flag.
///          - User needs to retrieve all records with `MY_SPECIAL_TYPE` value of user-defined enumeration.
///
///          Works as a handle to real signal representation instance.
///          Prevents destruction unless there is only one reference to instance.
///
/// \note Performance considerations
///       In most cases, signaled to unsignaled transaction is done only from special cycles, that process all signaled
///       records. It means that implementations are highly likely to be designed to process signaled to unsignaled
///       transaction when edition was done through SignalRepresentation::EditCursor as fast as possible while
///       sacrificing performance for the cases when this change is done through other means.
class RecordCollectionApi SignalRepresentation final
{
public:
    /// \brief Allows user to read signaled records.
    /// \details All ReadCursor operations are thread safe.
    class RecordCollectionApi ReadCursor final
    {
    public:
        EMERGENCE_READ_CURSOR_OPERATIONS (ReadCursor);

    private:
        /// SignalRepresentation constructs its cursors.
        friend class SignalRepresentation;

        EMERGENCE_BIND_IMPLEMENTATION_INPLACE (sizeof (std::uintptr_t) * 2u);

        explicit ReadCursor (std::array<std::uint8_t, DATA_MAX_SIZE> &_data) noexcept;
    };

    /// \brief Allows user to read, modify and delete signaled records.
    class RecordCollectionApi EditCursor final
    {
    public:
        EMERGENCE_EDIT_CURSOR_OPERATIONS (EditCursor);

    private:
        /// SignalRepresentation constructs its cursors.
        friend class SignalRepresentation;

        EMERGENCE_BIND_IMPLEMENTATION_INPLACE (sizeof (std::uintptr_t) * 2u);

        explicit EditCursor (std::array<std::uint8_t, DATA_MAX_SIZE> &_data) noexcept;
    };

    SignalRepresentation (const SignalRepresentation &_other) noexcept;

    SignalRepresentation (SignalRepresentation &&_other) noexcept;

    ~SignalRepresentation () noexcept;

    /// \brief Allows user to read signaled records.
    /// \details Complexity -- O(1). Order is not guaranteed and should be considered random.
    /// \invariant There is no active allocation transactions and edit cursors in Collection.
    ReadCursor ReadSignaled () noexcept;

    /// \brief Allows user to read, modify and delete signaled records.
    /// \details Complexity -- O(1). Order is not guaranteed and should be considered random.
    /// \invariant There is no active allocation transactions and read or edit cursors in Collection.
    EditCursor EditSignaled () noexcept;

    /// \return Field, which value will be checked for signaled-unsignaled state.
    [[nodiscard]] StandardLayout::Field GetKeyField () const noexcept;

    /// \return Whether given value is equal to representation signaled value.
    /// \details This method omits all insignificant bits from both signaled value and given value to make sure
    ///          that comparison is logically correct. Insignificant unequal bits usually arise due to `array_cast`
    ///          usage, in other words these bits are garbage that was accidentally captured from memory slice.
    [[nodiscard]] bool IsSignaledValue (const std::array<std::uint8_t, sizeof (std::uint64_t)> &_value) const;

    /// \return Signaled value.
    /// \warning Use ::IsSignaledValue for comparison.
    /// \details We return value instead of reference, because implementation
    ///          might store signaled value in custom optimized format.
    [[nodiscard]] std::array<std::uint8_t, sizeof (std::uint64_t)> GetSignaledValue () const;

    /// \see Collection::GetRecordMapping
    [[nodiscard]] const StandardLayout::Mapping &GetTypeMapping () const noexcept;

    /// \return Can this representation be safely dropped?
    /// \details Representation can be safely dropped if there is only one reference to it and there is no active
    ///          cursors.
    [[nodiscard]] bool CanBeDropped () const noexcept;

    /// \brief Deletes this signal representation from Collection.
    /// \invariant ::CanBeDropped
    void Drop () noexcept;

    /// \return True if this and given instances are handles to the same representation.
    bool operator== (const SignalRepresentation &_other) const noexcept;

    SignalRepresentation &operator= (const SignalRepresentation &_other) noexcept;

    SignalRepresentation &operator= (SignalRepresentation &&_other) noexcept;

private:
    /// Collection constructs representations.
    friend class Collection;

    explicit SignalRepresentation (void *_handle) noexcept;

    EMERGENCE_BIND_IMPLEMENTATION_HANDLE ()
};
} // namespace Emergence::RecordCollection

#pragma once

#include <array>
#include <cstdint>

#include <StandardLayout/Field.hpp>

namespace Emergence::RecordCollection
{
class LinearResolver final
{
public:
    /// \brief Allows user to read records, that match criteria, specified in LinearResolver::ReadInterval.
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
        /// LinearResolver constructs its cursors.
        friend class LinearResolver;

        static constexpr std::size_t DATA_MAX_SIZE = sizeof (uintptr_t) * 2u;

        explicit EditCursor (const std::array <uint8_t, DATA_MAX_SIZE> *_data) noexcept;

        /// \brief Iterator implementation-specific data.
        std::array <uint8_t, DATA_MAX_SIZE> data;
    };

    LinearResolver (const LinearResolver &_other) noexcept;

    LinearResolver (LinearResolver &&_other) noexcept;

    ~LinearResolver () noexcept;

    ReadCursor ReadInterval (const uint8_t *_keyFieldMinValue, const uint8_t *_keyFieldMaxValue) noexcept;

    EditCursor EditInterval (const uint8_t *_keyFieldMinValue, const uint8_t *_keyFieldMaxValue) noexcept;

    StandardLayout::Field GetKeyField () const noexcept;

    bool CanBeDropped () const;

    void Drop ();

private:
    /// Collection constructs resolvers.
    friend class Collection;

    explicit LinearResolver (void *_handle) noexcept;

    /// \brief Implementation handle.
    void *handle;
};
} // namespace Emergence::RecordCollection

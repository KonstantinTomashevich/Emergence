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

        // TODO: Is operator-only cursor API good or bad?

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

    PointResolver (const PointResolver &_other) noexcept;

    PointResolver (PointResolver &&_other) noexcept;

    ~PointResolver () noexcept;

    ReadCursor ReadPoint (const uint8_t *_keyFieldValues) noexcept;

    EditCursor EditPoint (const uint8_t *_keyFieldValues) noexcept;

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

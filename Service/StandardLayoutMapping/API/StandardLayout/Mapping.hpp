#pragma once

#include <array>
#include <cstdint>

#include <StandardLayout/Field.hpp>

namespace Emergence::StandardLayout
{
/// \brief Finished field mapping for user defined object type.
///
/// \details MappingBuilder should be used to construct mappings.
class Mapping final
{
public:
    /// \brief Allows iteration over Mapping fields.
    class FieldIterator final
    {
    public:
        ~FieldIterator () noexcept;

        /// \return Field, to which iterator points.
        /// \invariant Inside valid bounds, but not in the ending.
        Field operator * () const noexcept;

        /// \brief Move to next field.
        /// \invariant Inside valid bounds, but not in the ending.
        FieldIterator &operator ++ () noexcept;

        /// \brief Move to next field.
        /// \return Unchanged instance of iterator.
        /// \invariant Inside valid bounds, but not in the ending.
        FieldIterator operator ++ (int) noexcept;

        /// \brief Move to previous field.
        /// \invariant Inside valid bounds, but not in the beginning.
        FieldIterator &operator -- () noexcept;

        /// \brief Move to previous field.
        /// \return Unchanged instance of iterator.
        /// \invariant Inside valid bounds, but not in the beginning.
        FieldIterator operator -- (int) noexcept;

        bool operator == (const FieldIterator &_other) const noexcept;

        bool operator != (const FieldIterator &_other) const noexcept;

    private:
        /// Mapping constructs iterators,
        friend class Mapping;

        static constexpr std::size_t DATA_MAX_SIZE = sizeof (uintptr_t);

        explicit FieldIterator (const std::array <uint8_t, DATA_MAX_SIZE> *_data) noexcept;

        /// \brief Iterator implementation-specific data.
        std::array <uint8_t, DATA_MAX_SIZE> data;
    };

    Mapping (const Mapping &_other) noexcept;

    Mapping (Mapping &&_other) noexcept;

    ~Mapping () noexcept;

    /// \return User defined object size in bytes including alignment gaps.
    std::size_t GetObjectSize () const noexcept;

    /// \return Pointer to meta of field with given id or `nullptr` if there is no such field.
    Field GetField (FieldId _field) const noexcept;

    /// \return Iterator, that points to beginning of fields range.
    FieldIterator Begin () const noexcept;

    /// \return Iterator, that points to ending of fields range.
    FieldIterator End () const noexcept;

    /// \return Id of field, to which iterator points.
    /// \invariant Inside valid bounds, but not in the ending.
    FieldId GetFieldId (const FieldIterator &_iterator) const noexcept;

private:
    /// Mapping builder constructs mappings.
    friend class MappingBuilder;

    /// Field::GetNestedObjectMapping() wraps implementation data into Mapping interface.
    friend class Field;

    explicit Mapping (void *_handle) noexcept;

    /// \brief Mapping implementation handle.
    void *handle = nullptr;
};

/// \brief Wraps Mapping::Begin for foreach sentences.
Mapping::FieldIterator begin (const Mapping &_mapping) noexcept;

/// \brief Wraps Mapping::End for foreach sentences.
Mapping::FieldIterator end (const Mapping &_mapping) noexcept;
} // namespace Emergence::StandardLayout

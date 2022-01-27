#pragma once

#include <array>
#include <cstdint>

#include <API/Common/ImplementationBinding.hpp>
#include <API/Common/Iterator.hpp>
#include <API/Common/Shortcuts.hpp>

#include <Memory/UniqueString.hpp>

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
        EMERGENCE_BIDIRECTIONAL_ITERATOR_OPERATIONS (FieldIterator, Field);

    private:
        /// Mapping constructs iterators.
        friend class Mapping;

        EMERGENCE_BIND_IMPLEMENTATION_INPLACE (sizeof (uintptr_t));

        explicit FieldIterator (const std::array<uint8_t, DATA_MAX_SIZE> *_data) noexcept;
    };

    Mapping (const Mapping &_other) noexcept;

    Mapping (Mapping &&_other) noexcept;

    ~Mapping () noexcept;

    /// \return User defined object size in bytes including alignment gaps.
    [[nodiscard]] std::size_t GetObjectSize () const noexcept;

    /// \return Readable name for object type, described by this mapping.
    [[nodiscard]] Memory::UniqueString GetName () const noexcept;

    /// \brief If mapping has default constructor, executes it at given address.
    void Construct (void *_address) const noexcept;

    /// \brief If mapping has default destructor, executes it at given address.
    void Destruct (void *_address) const noexcept;

    /// \return Pointer to meta of field with given id or `nullptr` if there is no such field.
    [[nodiscard]] Field GetField (FieldId _field) const noexcept;

    /// \return Iterator, that points to beginning of fields range.
    [[nodiscard]] FieldIterator Begin () const noexcept;

    /// \return Iterator, that points to ending of fields range.
    [[nodiscard]] FieldIterator End () const noexcept;

    /// \return Id of field, to which iterator points.
    /// \invariant Inside valid bounds, but not in the ending.
    [[nodiscard]] FieldId GetFieldId (const FieldIterator &_iterator) const noexcept;

    /// \return Id of given field.
    /// \invariant Given field belongs to this mapping.
    [[nodiscard]] FieldId GetFieldId (const Field &_field) const noexcept;

    /// \warning If two mappings were built independently for the same type, behaviour is implementation-defined.
    bool operator== (const Mapping &_other) const noexcept;

    /// \warning If two mappings were built independently for the same type, behaviour is implementation-defined.
    bool operator!= (const Mapping &_other) const noexcept;

    /// Assigning mappings looks counter intuitive.
    EMERGENCE_DELETE_ASSIGNMENT (Mapping);

private:
    /// Mapping builder constructs mappings.
    friend class MappingBuilder;

    /// Field::GetNestedObjectMapping() wraps implementation data into Mapping interface.
    friend class Field;

    EMERGENCE_BIND_IMPLEMENTATION_INPLACE (sizeof (uintptr_t));

    /// \brief Copies implementation-specific values from given pointer.
    explicit Mapping (const std::array<uint8_t, DATA_MAX_SIZE> *_data) noexcept;

    /// \brief Moves implementation-specific values from given pointer.
    explicit Mapping (std::array<uint8_t, DATA_MAX_SIZE> *_data) noexcept;
};

/// \brief Wraps Mapping::Begin for foreach sentences.
Mapping::FieldIterator begin (const Mapping &_mapping) noexcept;

/// \brief Wraps Mapping::End for foreach sentences.
Mapping::FieldIterator end (const Mapping &_mapping) noexcept;
} // namespace Emergence::StandardLayout

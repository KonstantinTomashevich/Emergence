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
// TODO: We need special representation for InplaceVector's and unions for serialization.
//       There is two possible solutions to this:
//       - Store this data inside each field, so it will be accessible everywhere, but would eat a lot of memory.
//       - Store markers between fields (UNION_BEGIN, UNION_SWITCH, UNION_END, etc). Uses a lot less memory,
//         but can be extracted only through iteration.

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

        explicit FieldIterator (const std::array<uint8_t, DATA_MAX_SIZE> &_data) noexcept;
    };

    /// \brief Allows iteration over Mapping fields that are visible from given object.
    /// \details Object class may contain unions and inplace vectors, therefore some fields might be invisible due to
    ///          object values. For example, if class has union field and union variant specifier is `1` then only
    ///          fields from variant associated with `1` are visible because other union fields just have no logical
    ///          meaning for that object.
    class ConditionalFieldIterator final
    {
    public:
        EMERGENCE_FORWARD_ITERATOR_OPERATIONS (ConditionalFieldIterator, Field);

    private:
        /// Mapping constructs iterators.
        friend class Mapping;

        EMERGENCE_BIND_IMPLEMENTATION_INPLACE (sizeof (uintptr_t) * 6u);

        explicit ConditionalFieldIterator (const std::array<uint8_t, DATA_MAX_SIZE> &_data) noexcept;
    };

    /// \brief Constructs invalid mapping. Needed only to simplify registration procedure and must be reassigned.
    Mapping () noexcept;

    Mapping (const Mapping &_other) noexcept;

    Mapping (Mapping &&_other) noexcept;

    ~Mapping () noexcept;

    /// \return User defined object size in bytes including alignment gaps.
    [[nodiscard]] std::size_t GetObjectSize () const noexcept;

    /// \return User defined object alignment in bytes.
    [[nodiscard]] std::size_t GetObjectAlignment () const noexcept;

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

    /// \brief Like ::Begin, but with conditional visibility.
    [[nodiscard]] ConditionalFieldIterator BeginConditional (const void *_object) const noexcept;

    /// \brief Like ::End, but with conditional visibility.
    [[nodiscard]] ConditionalFieldIterator EndConditional () const noexcept;

    /// \return Id of field, to which iterator points.
    /// \invariant Inside valid bounds, but not in the ending.
    [[nodiscard]] FieldId GetFieldId (const FieldIterator &_iterator) const noexcept;

    /// \return Id of given field.
    /// \invariant Given field belongs to this mapping.
    [[nodiscard]] FieldId GetFieldId (const Field &_field) const noexcept;

    /// \return Hash for this mapping.
    [[nodiscard]] uintptr_t Hash () const noexcept;

    /// \warning If two mappings were built independently for the same type, behaviour is implementation-defined.
    bool operator== (const Mapping &_other) const noexcept;

    /// \warning If two mappings were built independently for the same type, behaviour is implementation-defined.
    bool operator!= (const Mapping &_other) const noexcept;

    Mapping &operator= (const Mapping &_other) noexcept;

    Mapping &operator= (Mapping &&_other) noexcept;

private:
    /// Mapping builder constructs mappings.
    friend class MappingBuilder;

    /// Field::GetNestedObjectMapping() wraps implementation data into Mapping interface.
    friend class Field;

    EMERGENCE_BIND_IMPLEMENTATION_INPLACE (sizeof (uintptr_t));

    /// \brief Copies implementation-specific values from given pointer.
    explicit Mapping (const std::array<uint8_t, DATA_MAX_SIZE> &_data) noexcept;

    /// \brief Moves implementation-specific values from given pointer.
    explicit Mapping (std::array<uint8_t, DATA_MAX_SIZE> &_data) noexcept;
};

/// \brief Wraps Mapping::Begin for foreach sentences.
Mapping::FieldIterator begin (const Mapping &_mapping) noexcept;

/// \brief Wraps Mapping::End for foreach sentences.
Mapping::FieldIterator end (const Mapping &_mapping) noexcept;
} // namespace Emergence::StandardLayout

namespace std
{
template <>
struct hash<Emergence::StandardLayout::Mapping>
{
    std::size_t operator() (const Emergence::StandardLayout::Mapping &_mapping) const noexcept
    {
        return static_cast<std::size_t> (_mapping.Hash ());
    }
};
} // namespace std

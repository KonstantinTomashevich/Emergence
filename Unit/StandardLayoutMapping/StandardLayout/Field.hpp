#pragma once

#include <StandardLayoutMappingApi.hpp>

#include <cstdint>
#include <cstdlib>

#include <API/Common/ImplementationBinding.hpp>

#include <Memory/Heap.hpp>

namespace Emergence::StandardLayout
{
/// \brief Mapped field unique identifier.
using FieldId = std::uint_fast64_t;

/// \brief Defines mapping-independent field space projection rule.
///
/// \param _objectField Id of any field in class `Y`, that contains structure of class `X`.
/// \param _nestedField Id of any field from class `X`.
/// \return Unique id (among `Y` class fields) for `nestedField` of `X` from `objectField` of `Y`.
StandardLayoutMappingApi FieldId ProjectNestedField (FieldId _objectField, FieldId _nestedField) noexcept;

/// \brief Declares field archetype, that can be used to reconstruct actual field type.
///
/// \details Field type reconstruction can be useful for serialization or reflection-based comparison.
enum class FieldArchetype : std::uint8_t
{
    /// \brief Single bit.
    BIT = 0u,

    /// \brief Just integer.
    INT,

    /// \brief Unsigned integer.
    UINT,

    /// \brief Floating point number.
    FLOAT,

    /// \brief Zero terminated inplace string.
    STRING,

    /// \brief Fixed size memory block.
    BLOCK,

    /// \brief Memory::UniqueString instance.
    /// \details Special archetype for unique string is created because they can be hashed or compared in constant time.
    UNIQUE_STRING,

    /// \brief Complex field, that has internal fields.
    ///
    /// \details All nested fields are projected into root mapping, but sometimes
    ///          it's useful to process complex fields as independent nested objects.
    NESTED_OBJECT,

    /// \brief Instance of Container::Utf8String.
    /// \details In contrast to ::STRING, ::UTF8_STRING represents string in external address space.
    UTF8_STRING,

    /// \brief Instance of Emergence::Container::Vector that stores objects, that can be described by their mappings.
    /// \details Field projection is impossible here due to different and dynamic address space.
    ///          Therefore, this archetype primary goal is to make serialization easier.
    VECTOR,

    /// \brief Instance of Patch.
    /// \details Field projection is impossible here due to dynamic and unpredictable nature of patches.
    ///          Therefore, this archetype primary goal is to make serialization easier.
    PATCH,
};

/// \brief Projected field name is `objectFieldName + PROJECTION_NAME_SEPARATOR + nestedFieldName`.
constexpr char PROJECTION_NAME_SEPARATOR = '.';

/// \brief Provides read access to information about field.
///
/// \details Mapping of corresponding type should be used to get Field by FieldId.
/// \warning Storing Field instance outside of corresponding Mapping instance scope results in undefined behaviour.
class StandardLayoutMappingApi Field final
{
public:
    /// \brief Constructs field with invalid handle. Used to create temporary placeholder, for example to fill array.
    Field () noexcept;

    Field (const Field &_other) noexcept;

    Field (Field &&_other) noexcept;

    // NOLINTNEXTLINE(performance-trivially-destructible): Implementations may have non-trivial destructors.
    ~Field () noexcept;

    /// \return Field archetype, in pair with field size can be used to reconstruct actual field type.
    /// \invariant Handle must be valid.
    [[nodiscard]] FieldArchetype GetArchetype () const noexcept;

    /// \return Whether this field was added to the mapping through field projection.
    /// \invariant Handle must be valid.
    [[nodiscard]] bool IsProjected () const noexcept;

    /// \return Field offset in mapped structure in bytes.
    /// \invariant Handle must be valid.
    [[nodiscard]] std::size_t GetOffset () const noexcept;

    /// \return Field size in bytes, excluding suffix alignment gap, but including internal gaps.
    /// \invariant Handle must be valid.
    [[nodiscard]] std::size_t GetSize () const noexcept;

    /// \return Offset of required bit in byte, pointed by #offset. Always less than 8u.
    /// \invariant Handle must be valid.
    /// \invariant Field archetype is FieldArchetype::BIT.
    [[nodiscard]] std::size_t GetBitOffset () const noexcept;

    /// \return Mapping, that describes nested object fields.
    /// \invariant Handle must be valid.
    /// \invariant Field archetype is FieldArchetype::NESTED_OBJECT.
    [[nodiscard]] class Mapping GetNestedObjectMapping () const noexcept;

    /// \return Mapping, that describes vector items.
    /// \invariant Handle must be valid.
    /// \invariant Field archetype is FieldArchetype::VECTOR.
    [[nodiscard]] class Mapping GetVectorItemMapping () const noexcept;

    /// \return Human readable field name.
    /// \invariant Handle must be valid.
    [[nodiscard]] Memory::UniqueString GetName () const noexcept;

    /// \param _object pointer to structure, that contains this field.
    /// \return Pointer to this field in given structure.
    /// \invariant Handle must be valid.
    /// \invariant _object is not `nullptr`.
    [[nodiscard]] void *GetValue (void *_object) const noexcept;

    /// \brief Const version of ::GetValue(void *).
    [[nodiscard]] const void *GetValue (const void *_object) const noexcept;

    /// \return Is given field handle points to the same field of the same mapping as this field handle?
    /// \invariant Handle must be valid.
    [[nodiscard]] bool IsSame (const Field &_other) const noexcept;

    /// \return Is field ::handle valid?
    [[nodiscard]] bool IsHandleValid () const noexcept;

    /// \return ::IsHandleValid ()
    explicit operator bool () const noexcept;

    Field &operator= (const Field &_other) noexcept;

    Field &operator= (Field &&_other) noexcept;

private:
    /// Fields are constructed inside Mapping.
    friend class Mapping;

    explicit Field (void *_handle) noexcept;

    EMERGENCE_BIND_IMPLEMENTATION_HANDLE ()
};
} // namespace Emergence::StandardLayout

EMERGENCE_MEMORY_DEFAULT_ALLOCATION_GROUP (StandardLayoutMappingApi, Emergence::StandardLayout::FieldId)

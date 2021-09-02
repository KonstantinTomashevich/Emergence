#pragma once

#include <cstdint>

#include <API/Common/ImplementationBinding.hpp>

namespace Emergence::StandardLayout
{
/// \brief Mapped field unique identifier.
using FieldId = uint_fast64_t;

/// \brief Defines mapping-independent field space projection rule.
///
/// \param objectField Id of any field in class `Y`, that contains structure of class `X`.
/// \param nestedField Id of any field from class `X`.
/// \return Unique id (among `Y` class fields) for `nestedField` of `X` from `objectField` of `Y`.
FieldId ProjectNestedField (FieldId objectField, FieldId nestedField) noexcept;

/// \brief Declares field archetype, that can be used to reconstruct actual field type.
///
/// \details Field type reconstruction can be useful for serialization or reflection-based comparison.
enum class FieldArchetype
{
    /// \brief Single bit.
    BIT = 0u,

    /// \brief Just integer.
    INT,

    /// \brief Unsigned integer.
    UINT,

    /// \brief Floating point number.
    FLOAT,

    /// \brief Zero terminated string.
    STRING,

    /// \brief Fixed size memory block.
    BLOCK,

    /// \brief Complex field, that has internal fields.
    ///
    /// \details All nested fields are projected into root mapping, but sometimes
    ///          it's useful to process complex fields as independent nested objects.
    NESTED_OBJECT,
};

/// \brief Provides read access to information about field.
///
/// \details Mapping of corresponding type should be used to get Field by FieldId.
/// \warning Storing Field instance outside of corresponding Mapping instance scope results in undefined behaviour.
class Field final
{
public:
    /// \brief Constructs field with invalid handle. Used to create temporary placeholder, for example to fill array.
    Field () noexcept;

    Field (const Field &_other) noexcept;

    Field (Field &&_other) noexcept;

    ~Field () noexcept;

    /// \return Field archetype, in pair with field size can be used to reconstruct actual field type.
    /// \invariant Handle must be valid.
    FieldArchetype GetArchetype () const noexcept;

    /// \return Field offset in mapped structure in bytes.
    /// \invariant Handle must be valid.
    std::size_t GetOffset () const noexcept;

    /// \return Field size in bytes, excluding suffix alignment gap, but including internal gaps.
    /// \invariant Handle must be valid.
    std::size_t GetSize () const noexcept;

    /// \return Offset of required bit in byte, pointed by #offset. Always less than 8u.
    /// \invariant Handle must be valid.
    /// \invariant Field archetype is FieldArchetype::BIT.
    std::size_t GetBitOffset () const noexcept;

    /// \return Mapping, that describes nested object fields.
    /// \invariant Handle must be valid.
    /// \invariant Field archetype is FieldArchetype::NESTED_OBJECT.
    class Mapping GetNestedObjectMapping () const noexcept;

    /// \return Human readable field name.
    /// \invariant Handle must be valid.
    const char *GetName () const noexcept;

    /// \param _object pointer to structure, that contains this field.
    /// \return Pointer to this field in given structure.
    /// \invariant Handle must be valid.
    /// \invariant _object is not `nullptr`.
    void *GetValue (void *_object) const noexcept;

    /// \brief Const version of ::GetValue(void *).
    const void *GetValue (const void *_object) const noexcept;

    /// \return Is given field handle points to the same field of the same mapping as this field handle?
    /// \invariant Handle must be valid.
    bool IsSame (const Field &_other) const noexcept;

    /// \return Is field ::handle valid?
    bool IsHandleValid () const noexcept;

    /// \return ::IsHandleValid ()
    explicit operator bool () const noexcept;

    Field &operator= (const Field &_other) noexcept;

    Field &operator= (Field &&_other) noexcept;

private:
    /// Fields are constructed inside Mapping.
    friend class Mapping;

    explicit Field (void *_handle) noexcept;

    EMERGENCE_BIND_IMPLEMENTATION_HANDLE ();
};
} // namespace Emergence::StandardLayout

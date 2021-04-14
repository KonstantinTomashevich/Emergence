#pragma once

#include <cstdint>
#include <cstdlib>

namespace Emergence::StandardLayout
{
/// \brief Mapped field unique identifier.
using FieldId = uint_fast64_t;

/// \brief Defines mapping-independent field space projection rule.
///
/// \param objectField id of any field in class `Y`, that contains structure of class `X`.
/// \param nestedField id of any field from class `X`.
/// \return unique id (among `Y` class fields) for `nestedField` of `X` from `objectField` of `Y`.
FieldId ProjectNestedField (FieldId objectField, FieldId nestedField) noexcept;

/// \brief Declares field archetype, that can be used to reconstruct actual field type.
///
/// \details Field type reconstruction can be useful for serialization or reflection-based comparison.
enum class FieldArchetype
{
    /// Single bit.
    BIT,

    /// Just integer.
    INT,

    /// Unsigned integer.
    UINT,

    /// Floating point number.
    FLOAT,

    /// Zero terminated string.
    STRING,

    /// Fixed size memory block.
    BLOCK,

    // TODO: Archetype for nested fields? Save info about field type instead of size?
};

/// \brief Provides read access to information about field.
///
/// \details Mapping of corresponding type should be used to get Field by FieldId.
/// \warning Storing Field instance outside of corresponding Mapping instance scope results in undefined behaviour.
class Field final
{
public:
    /// \return Field archetype, in pair with field size can be used to reconstruct actual field type.
    FieldArchetype GetArchetype () const noexcept;

    /// \return Field offset in mapped structure in bytes.
    std::size_t GetOffset () const noexcept;

    /// \return Field size in bytes, excluding suffix alignment gap, but including internal gaps.
    std::size_t GetSize () const noexcept;

    /// \return Offset of required bit in byte, pointed by #offset. Always less than 8u.
    /// \invariant Field archetype is FieldArchetype::BIT.
    std::size_t GetBitOffset () const noexcept;

    /// \return Mapping of object type, which instance resides in this field.
    /// \invariant Field archetype is FieldArchetype::NESTED.
    class Mapping GetNestedMapping () const noexcept;

    /// \param _object pointer to structure, that contains this field.
    /// \return pointer to this field in given structure.
    /// \invariant _object is not `nullptr`.
    void *GetValue (void *_object) const noexcept;

    /// \brief Const version of ::GetValue(void *).
    const void *GetValue (const void *_object) const noexcept;

private:
    friend class Mapping;

    explicit Field (void *_handle);

    ~Field ();

    /// \brief Field implementation handle.
    void *handle;
};
} // namespace Emergence::StandardLayout

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

/// \brief Contains basic information about field.
struct FieldMeta final
{
public:
    /// \brief Field offset in mapped structure in bytes.
    std::size_t offset;

    union
    {
        /// \brief Field size in bytes, excluding suffix alignment gap, gut including internal gaps.
        ///
        /// \invariant #archetype is not FieldArchetype::BIT.
        std::size_t size;

        /// \brief Offset of required bit in byte, pointer by #offset.
        ///
        /// \invariant #archetype is FieldArchetype::BIT.
        /// \invariant less than 8u.
        uint_fast8_t bitOffset;
    };

    /// \brief Field archetype, in pair with #size can be used to reconstruct actual field type.
    FieldArchetype archetype;

    /// \param _object pointer to structure, that contains this field.
    /// \return pointer to this field in given structure.
    /// \invariant _object is not `nullptr`.
    void *GetValue (void *_object) const noexcept;

    /// \brief Const version of ::GetValue(void *).
    const void *GetValue (const void *_object) const noexcept;
};
} // namespace Emergence::StandardLayout

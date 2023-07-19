#pragma once

#include <StandardLayoutMappingApi.hpp>

#include <cstdint>

#include <API/Common/ImplementationBinding.hpp>

#include <StandardLayout/Patch.hpp>

namespace Emergence::StandardLayout
{
/// \brief Hides Patch building logic from user to prevent usage of unfinished patches.
class StandardLayoutMappingApi PatchBuilder final
{
public:
    /// \brief Automatically detects difference between given objects of given type and creates patch that
    ///        transforms objects from `_initial` into `_changed` state.
    /// \details Supported field archetypes:
    ///          * FieldArchetype::BIT
    ///          * FieldArchetype::INT
    ///          * FieldArchetype::UINT
    ///          * FieldArchetype::FLOAT
    ///          * FieldArchetype::UNIQUE_STRING
    ///          Nested fields are ignored, because their fields are projected.
    static Patch FromDifference (const Mapping &_typeMapping, const void *_changed, const void *_initial) noexcept;

    PatchBuilder () noexcept;

    PatchBuilder (const PatchBuilder &_other) = delete;

    PatchBuilder (PatchBuilder &&_other) noexcept;

    ~PatchBuilder () noexcept;

    /// \brief Starts patch construction routine.
    /// \invariant There is no active patch construction routine that uses this builder.
    void Begin (const Mapping &_typeMapping) noexcept;

    /// \brief Finishes patch construction routine.
    /// \invariant There is active patch construction routine that uses this builder.
    Patch End () noexcept;

    /// \brief Pushes command that sets given value to given bit field.
    /// \invariant There is active patch construction routine that uses this builder.
    void SetBit (FieldId _field, bool _value) noexcept;

    /// \brief Pushes command that sets given value to given integer field.
    /// \invariant There is active patch construction routine that uses this builder.
    void SetInt8 (FieldId _field, int8_t _value) noexcept;

    /// \brief Pushes command that sets given value to given integer field.
    /// \invariant There is active patch construction routine that uses this builder.
    void SetInt16 (FieldId _field, int16_t _value) noexcept;

    /// \brief Pushes command that sets given value to given integer field.
    /// \invariant There is active patch construction routine that uses this builder.
    void SetInt32 (FieldId _field, int32_t _value) noexcept;

    /// \brief Pushes command that sets given value to given integer field.
    /// \invariant There is active patch construction routine that uses this builder.
    void SetInt64 (FieldId _field, int64_t _value) noexcept;

    /// \brief Pushes command that sets given value to given unsigned integer field.
    /// \invariant There is active patch construction routine that uses this builder.
    void SetUInt8 (FieldId _field, std::uint8_t _value) noexcept;

    /// \brief Pushes command that sets given value to given unsigned integer field.
    /// \invariant There is active patch construction routine that uses this builder.
    void SetUInt16 (FieldId _field, std::uint16_t _value) noexcept;

    /// \brief Pushes command that sets given value to given unsigned integer field.
    /// \invariant There is active patch construction routine that uses this builder.
    void SetUInt32 (FieldId _field, std::uint32_t _value) noexcept;

    /// \brief Pushes command that sets given value to given unsigned integer field.
    /// \invariant There is active patch construction routine that uses this builder.
    void SetUInt64 (FieldId _field, std::uint64_t _value) noexcept;

    /// \brief Pushes command that sets given value to given floating point field.
    /// \invariant There is active patch construction routine that uses this builder.
    void SetFloat (FieldId _field, float _value) noexcept;

    /// \brief Pushes command that sets given value to given floating point field.
    /// \invariant There is active patch construction routine that uses this builder.
    void SetDouble (FieldId _field, double _value) noexcept;

    /// \brief Pushes command that sets given value to given unique string field.
    /// \invariant There is active patch construction routine that uses this builder.
    void SetUniqueString (FieldId _field, const Memory::UniqueString &_value) noexcept;

    /// Assigning patch builders looks counter-intuitive.
    EMERGENCE_DELETE_ASSIGNMENT (PatchBuilder);

private:
    EMERGENCE_BIND_IMPLEMENTATION_INPLACE (sizeof (std::uintptr_t));
};
} // namespace Emergence::StandardLayout

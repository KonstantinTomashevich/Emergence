#pragma once

#include <cstdint>

#include <StandardLayout/Mapping.hpp>

namespace Emergence::StandardLayout
{
/// \brief Hides Mapping building logic from user to prevent usage of unfinished mappings.
class MappingBuilder final
{
public:
    MappingBuilder () noexcept;

    ~MappingBuilder ();

    /// \brief Starts mapping construction routine.
    /// \invariant There is no active mapping construction routine, that uses this builder.
    void Begin (std::size_t _objectSize) noexcept;

    /// \brief Finishes mapping construction routine.
    /// \invariant There is active mapping construction routine, that uses this builder.
    Mapping End () noexcept;

    /// \brief Registers bit flag.
    /// \invariant There is active mapping construction routine, that uses this builder.
    FieldId RegisterBit (std::size_t _offset, uint_fast8_t _bitOffset) noexcept;

    /// \brief Registers 1 byte long integer.
    /// \invariant There is active mapping construction routine, that uses this builder.
    FieldId RegisterInt8 (std::size_t _offset) noexcept;

    /// \brief Registers 2 bytes long integer.
    /// \invariant There is active mapping construction routine, that uses this builder.
    FieldId RegisterInt16 (std::size_t _offset) noexcept;

    /// \brief Registers 4 bytes long integer.
    /// \invariant There is active mapping construction routine, that uses this builder.
    FieldId RegisterInt32 (std::size_t _offset) noexcept;

    /// \brief Registers 8 bytes long integer.
    /// \invariant There is active mapping construction routine, that uses this builder.
    FieldId RegisterInt64 (std::size_t _offset) noexcept;

    /// \brief Registers 1 byte long unsigned integer.
    /// \invariant There is active mapping construction routine, that uses this builder.
    FieldId RegisterUInt8 (std::size_t _offset) noexcept;

    /// \brief Registers 2 bytes long unsigned integer.
    /// \invariant There is active mapping construction routine, that uses this builder.
    FieldId RegisterUInt16 (std::size_t _offset) noexcept;

    /// \brief Registers 4 bytes long unsigned integer.
    /// \invariant There is active mapping construction routine, that uses this builder.
    FieldId RegisterUInt32 (std::size_t _offset) noexcept;

    /// \brief Registers 8 bytes long unsigned integer.
    /// \invariant There is active mapping construction routine, that uses this builder.
    FieldId RegisterUInt64 (std::size_t _offset) noexcept;

    /// \brief Registers `sizeof (float)` bytes long floating point number.
    /// \invariant There is active mapping construction routine, that uses this builder.
    FieldId RegisterFloat (std::size_t _offset) noexcept;

    /// \brief Registers `sizeof (double)` bytes long floating point number.
    /// \invariant There is active mapping construction routine, that uses this builder.
    FieldId RegisterDouble (std::size_t _offset) noexcept;

    /// \brief Registers zero terminated string with fixed maximum size.
    /// \invariant There is active mapping construction routine, that uses this builder.
    FieldId RegisterString (std::size_t _offset, std::size_t _maxSize) noexcept;

    /// \brief Registers memory block with given size.
    /// \invariant There is active mapping construction routine, that uses this builder.
    FieldId RegisterBlock (std::size_t _offset, std::size_t _size) noexcept;

    /// \brief Registers nested structure using its Mapping.
    /// \invariant There is active mapping construction routine, that uses this builder.
    FieldId RegisterNestedObject (std::size_t _offset, const Mapping &objectMapping) noexcept;

private:
    /// \brief MappingBuilder implementation handle.
    void *handle = nullptr;
};
} // namespace Emergence::StandardLayout

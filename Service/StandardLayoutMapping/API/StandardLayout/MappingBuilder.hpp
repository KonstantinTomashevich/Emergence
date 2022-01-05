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

    /// MappingBuilder is used as Mapping construction helper, there it sounds irrational to copy it.
    MappingBuilder (const MappingBuilder &_other) = delete;

    MappingBuilder (MappingBuilder &&_other) noexcept;

    ~MappingBuilder () noexcept;

    /// \brief Starts mapping construction routine.
    /// \invariant There is no active mapping construction routine, that uses this builder.
    void Begin (Memory::UniqueString _name, std::size_t _objectSize) noexcept;

    /// \brief Finishes mapping construction routine.
    /// \invariant There is active mapping construction routine, that uses this builder.
    [[nodiscard]] Mapping End () noexcept;

    /// \brief Registers bit flag.
    /// \invariant There is active mapping construction routine, that uses this builder.
    [[nodiscard]] FieldId RegisterBit (Memory::UniqueString _name,
                                       std::size_t _offset,
                                       uint_fast8_t _bitOffset) noexcept;

    /// \brief Registers 1 byte long integer.
    /// \invariant There is active mapping construction routine, that uses this builder.
    [[nodiscard]] FieldId RegisterInt8 (Memory::UniqueString _name, std::size_t _offset) noexcept;

    /// \brief Registers 2 bytes long integer.
    /// \invariant There is active mapping construction routine, that uses this builder.
    [[nodiscard]] FieldId RegisterInt16 (Memory::UniqueString _name, std::size_t _offset) noexcept;

    /// \brief Registers 4 bytes long integer.
    /// \invariant There is active mapping construction routine, that uses this builder.
    [[nodiscard]] FieldId RegisterInt32 (Memory::UniqueString _name, std::size_t _offset) noexcept;

    /// \brief Registers 8 bytes long integer.
    /// \invariant There is active mapping construction routine, that uses this builder.
    [[nodiscard]] FieldId RegisterInt64 (Memory::UniqueString _name, std::size_t _offset) noexcept;

    /// \brief Registers 1 byte long unsigned integer.
    /// \invariant There is active mapping construction routine, that uses this builder.
    [[nodiscard]] FieldId RegisterUInt8 (Memory::UniqueString _name, std::size_t _offset) noexcept;

    /// \brief Registers 2 bytes long unsigned integer.
    /// \invariant There is active mapping construction routine, that uses this builder.
    [[nodiscard]] FieldId RegisterUInt16 (Memory::UniqueString _name, std::size_t _offset) noexcept;

    /// \brief Registers 4 bytes long unsigned integer.
    /// \invariant There is active mapping construction routine, that uses this builder.
    [[nodiscard]] FieldId RegisterUInt32 (Memory::UniqueString _name, std::size_t _offset) noexcept;

    /// \brief Registers 8 bytes long unsigned integer.
    /// \invariant There is active mapping construction routine, that uses this builder.
    [[nodiscard]] FieldId RegisterUInt64 (Memory::UniqueString _name, std::size_t _offset) noexcept;

    /// \brief Registers `sizeof (float)` bytes long floating point number.
    /// \invariant There is active mapping construction routine, that uses this builder.
    [[nodiscard]] FieldId RegisterFloat (Memory::UniqueString _name, std::size_t _offset) noexcept;

    /// \brief Registers `sizeof (double)` bytes long floating point number.
    /// \invariant There is active mapping construction routine, that uses this builder.
    [[nodiscard]] FieldId RegisterDouble (Memory::UniqueString _name, std::size_t _offset) noexcept;

    /// \brief Registers zero terminated string with fixed maximum size.
    /// \invariant There is active mapping construction routine, that uses this builder.
    [[nodiscard]] FieldId RegisterString (Memory::UniqueString _name,
                                          std::size_t _offset,
                                          std::size_t _maxSize) noexcept;

    /// \brief Registers memory block with given size.
    /// \invariant There is active mapping construction routine, that uses this builder.
    [[nodiscard]] FieldId RegisterBlock (Memory::UniqueString _name, std::size_t _offset, std::size_t _size) noexcept;

    /// \brief Registers Memory::UniqueString field.
    /// \invariant There is active mapping construction routine, that uses this builder.
    [[nodiscard]] FieldId RegisterUniqueString (Memory::UniqueString _name, std::size_t _offset) noexcept;

    /// \brief Registers nested object using its Mapping.
    /// \invariant There is active mapping construction routine, that uses this builder.
    [[nodiscard]] FieldId RegisterNestedObject (Memory::UniqueString _name,
                                                std::size_t _offset,
                                                const Mapping &_objectMapping) noexcept;

    MappingBuilder &operator= (const MappingBuilder &_other) = delete;

    MappingBuilder &operator= (MappingBuilder &&_other) noexcept;

private:
    EMERGENCE_BIND_IMPLEMENTATION_HANDLE ();
};
} // namespace Emergence::StandardLayout

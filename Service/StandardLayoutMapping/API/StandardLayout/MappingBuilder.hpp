#pragma once

#include <cstdint>

#include <StandardLayout/Mapping.hpp>

namespace Emergence::StandardLayout
{
/// \brief Lists supported operations for MappingBuilder::PushVisibilityCondition.
enum class ConditionalOperation
{
    /// \brief True if field value is equal to argument.
    EQUAL = 0u,

    /// \brief True if field value is less than argument.
    LESS,

    /// \brief True if field value is greater than argument.
    GREATER,
};

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
    void Begin (Memory::UniqueString _name, std::size_t _objectSize, std::size_t _objectAlignment) noexcept;

    /// \brief Finishes mapping construction routine.
    /// \invariant There is active mapping construction routine that uses this builder.
    [[nodiscard]] Mapping End () noexcept;

    /// \brief Sets default constructor for objects of constructed mapping.
    /// \invariant There is active mapping construction routine that uses this builder.
    void SetConstructor (void (*_constructor) (void *)) noexcept;

    /// \brief Sets default move constructor for objects of constructed mapping.
    /// \invariant There is active mapping construction routine that uses this builder.
    void SetMoveConstructor (void (*_constructor) (void *, void *)) noexcept;

    /// \brief Sets default destructor for objects of constructed mapping.
    /// \invariant There is active mapping construction routine that uses this builder.
    void SetDestructor (void (*_destructor) (void *)) noexcept;

    /// \brief Registers bit flag.
    /// \invariant There is active mapping construction routine, that uses this builder.
    [[nodiscard]] FieldId RegisterBit (Memory::UniqueString _name,
                                       std::size_t _offset,
                                       std::uint_fast8_t _bitOffset) noexcept;

    /// \brief Registers 1 byte long integer.
    /// \invariant There is active mapping construction routine that uses this builder.
    [[nodiscard]] FieldId RegisterInt8 (Memory::UniqueString _name, std::size_t _offset) noexcept;

    /// \brief Registers 2 bytes long integer.
    /// \invariant There is active mapping construction routine that uses this builder.
    [[nodiscard]] FieldId RegisterInt16 (Memory::UniqueString _name, std::size_t _offset) noexcept;

    /// \brief Registers 4 bytes long integer.
    /// \invariant There is active mapping construction routine that uses this builder.
    [[nodiscard]] FieldId RegisterInt32 (Memory::UniqueString _name, std::size_t _offset) noexcept;

    /// \brief Registers 8 bytes long integer.
    /// \invariant There is active mapping construction routine that uses this builder.
    [[nodiscard]] FieldId RegisterInt64 (Memory::UniqueString _name, std::size_t _offset) noexcept;

    /// \brief Registers 1 byte long unsigned integer.
    /// \invariant There is active mapping construction routine that uses this builder.
    [[nodiscard]] FieldId RegisterUInt8 (Memory::UniqueString _name, std::size_t _offset) noexcept;

    /// \brief Registers 2 bytes long unsigned integer.
    /// \invariant There is active mapping construction routine that uses this builder.
    [[nodiscard]] FieldId RegisterUInt16 (Memory::UniqueString _name, std::size_t _offset) noexcept;

    /// \brief Registers 4 bytes long unsigned integer.
    /// \invariant There is active mapping construction routine that uses this builder.
    [[nodiscard]] FieldId RegisterUInt32 (Memory::UniqueString _name, std::size_t _offset) noexcept;

    /// \brief Registers 8 bytes long unsigned integer.
    /// \invariant There is active mapping construction routine that uses this builder.
    [[nodiscard]] FieldId RegisterUInt64 (Memory::UniqueString _name, std::size_t _offset) noexcept;

    /// \brief Registers `sizeof (float)` bytes long floating point number.
    /// \invariant There is active mapping construction routine that uses this builder.
    [[nodiscard]] FieldId RegisterFloat (Memory::UniqueString _name, std::size_t _offset) noexcept;

    /// \brief Registers `sizeof (double)` bytes long floating point number.
    /// \invariant There is active mapping construction routine that uses this builder.
    [[nodiscard]] FieldId RegisterDouble (Memory::UniqueString _name, std::size_t _offset) noexcept;

    /// \brief Registers zero terminated string with fixed maximum size.
    /// \invariant There is active mapping construction routine, that uses this builder.
    [[nodiscard]] FieldId RegisterString (Memory::UniqueString _name,
                                          std::size_t _offset,
                                          std::size_t _maxSize) noexcept;

    /// \brief Registers memory block with given size.
    /// \invariant There is active mapping construction routine that uses this builder.
    [[nodiscard]] FieldId RegisterBlock (Memory::UniqueString _name, std::size_t _offset, std::size_t _size) noexcept;

    /// \brief Registers Memory::UniqueString field.
    /// \invariant There is active mapping construction routine that uses this builder.
    [[nodiscard]] FieldId RegisterUniqueString (Memory::UniqueString _name, std::size_t _offset) noexcept;

    /// \brief Registers nested object using its Mapping.
    /// \invariant There is active mapping construction routine that uses this builder.
    [[nodiscard]] FieldId RegisterNestedObject (Memory::UniqueString _name,
                                                std::size_t _offset,
                                                const Mapping &_objectMapping) noexcept;

    /// \brief Registers Container::Utf8String field.
    /// \invariant There is active mapping construction routine that uses this builder.
    [[nodiscard]] FieldId RegisterUtf8String (Memory::UniqueString _name, std::size_t _offset) noexcept;

    /// \brief Registers vector that contains items of given Mapping.
    /// \invariant There is active mapping construction routine that uses this builder.
    [[nodiscard]] FieldId RegisterVector (Memory::UniqueString _name,
                                          std::size_t _offset,
                                          const Mapping &_itemMapping) noexcept;

    /// \brief Registers Patch field.
    /// \invariant There is active mapping construction routine that uses this builder.
    [[nodiscard]] FieldId RegisterPatch (Memory::UniqueString _name, std::size_t _offset) noexcept;

    /// \brief Pushes new condition to visibility conditions stack.
    /// \see Mapping::ConditionalFieldIterator
    /// \invariant `_field` is FieldArchetype::UINT.
    void PushVisibilityCondition (FieldId _field, ConditionalOperation _operation, std::uint64_t _argument) noexcept;

    /// \brief Pops current top condition from visibility condition stack.
    /// \invariant Visibility condition stack is not empty.
    void PopVisibilityCondition () noexcept;

    MappingBuilder &operator= (const MappingBuilder &_other) = delete;

    MappingBuilder &operator= (MappingBuilder &&_other) noexcept;

private:
    EMERGENCE_BIND_IMPLEMENTATION_INPLACE (sizeof (std::uintptr_t) * 3u);
};
} // namespace Emergence::StandardLayout

#pragma once

#include <array>

/// \brief Reinterprets given memory block as object of given type.
/// \details Syntax sugar for array-to-object casting.
///
/// \tparam Target type of object, that should be stored in given memory block.
/// \tparam _blockSize size of given memory block, usually inferred from _memoryBlock parameter.
/// \param _memoryBlock memory block array representation, that should contain object of _Target type.
/// \invariant sizeof (_Target) <= _blockSize.
///
/// \return _memoryBlock data, reinterpreted as object of _Target type.
template <typename Target, std::size_t _blockSize>
const Target &block_cast (const std::array<uint8_t, _blockSize> &_memoryBlock)
{
    // NOLINTNEXTLINE(bugprone-sizeof-expression): Target could be pointer and CLang Tidy flags `sizeof(pointer)`.
    static_assert (sizeof (Target) <= _blockSize);
    return *reinterpret_cast<const Target *> (&_memoryBlock);
}

/// \see Const version of this cast function.
template <typename Target, std::size_t _blockSize>
Target &block_cast (std::array<uint8_t, _blockSize> &_memoryBlock)
{
    return const_cast<Target &> (
        block_cast<Target> (const_cast<const std::array<uint8_t, _blockSize> &> (_memoryBlock)));
}

/// \brief Reinterprets given object as memory block, represented by array.
/// \details Syntax-sugar, opposite to ::block_cast.
///
/// \tparam Source Type of reinterpreted object.
/// \tparam Size Size of reinterpreted array. By default it is equal to object size.
/// \param _source Reference to object, that will be reinterpreted.
/// \return _source data, reinterpreted as array.
template <typename Source, size_t Size = sizeof (Source)>
const std::array<uint8_t, Size> &array_cast (const Source &_source)
{
    return *reinterpret_cast<const std::array<uint8_t, Size> *> (&_source);
}

/// \see Const version of this cast function.
template <typename Source, size_t Size = sizeof (Source)>
std::array<uint8_t, Size> &array_cast (Source &_source)
{
    return const_cast<std::array<uint8_t, Size> &> (array_cast<Source, Size> (const_cast<const Source &> (_source)));
}

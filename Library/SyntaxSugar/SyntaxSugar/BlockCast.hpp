#pragma once

#include <array>

/// \brief Reinterprets given memory block as object of given type.
/// \details Syntax sugar for array-to-object casting.
///
/// \tparam _Target type of object, that should be stored in given memory block.
/// \tparam _blockSize size of given memory block, usually inferred from _memoryBlock parameter.
/// \param _memoryBlock memory block array representation, that should contain object of _Target type.
/// \invariant sizeof (_Target) <= _blockSize.
///
/// \return _memoryBlock data, reinterpreted as object of _Target type.
template <typename _Target, std::size_t _blockSize>
const _Target &block_cast (const std::array<uint8_t, _blockSize> &_memoryBlock)
{
    static_assert (sizeof (_Target) <= _blockSize);
    return *reinterpret_cast<const _Target *> (&_memoryBlock);
}

/// \see Const version of this cast function.
template <typename _Target, std::size_t _blockSize>
_Target &block_cast (std::array<uint8_t, _blockSize> &_memoryBlock)
{
    return const_cast<_Target &> (
        block_cast<_Target> (const_cast<const std::array<uint8_t, _blockSize> &> (_memoryBlock)));
}
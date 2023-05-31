#pragma once

#include <cstdint>
#include <cstdlib>

namespace Emergence::Memory::Original
{
// We don't use const below, because going through a lot of consts with pointers is painful.

void *AlignedAllocate (std::size_t _alignment, std::size_t _amount) noexcept;

void *AlignedReallocate (void *_block, std::size_t _alignment, std::size_t _oldSize, std::size_t _newSize) noexcept;

void AlignedFree (void *_block) noexcept;

std::size_t CorrectAlignedBlockSize (std::size_t _alignment, std::size_t _requestedBlockSize) noexcept;

std::size_t GetPageSize (std::size_t _chunkSize, std::size_t _capacity) noexcept;

using AlignedPoolPage = void;

void *GetPageChunksBegin (AlignedPoolPage *_page) noexcept;

void *GetPageChunksEnd (AlignedPoolPage *_page, std::size_t _chunkSize, std::size_t _capacity) noexcept;

AlignedPoolPage *GetNextPagePointer (AlignedPoolPage *_page, std::size_t _chunkSize, std::size_t _capacity) noexcept;

void SetNextPagePointer (AlignedPoolPage *_page,
                         std::size_t _chunkSize,
                         std::size_t _capacity,
                         AlignedPoolPage *_next) noexcept;

constexpr std::size_t GetPageMetadataSize () noexcept
{
    return sizeof (std::uintptr_t);
}
} // namespace Emergence::Memory::Original

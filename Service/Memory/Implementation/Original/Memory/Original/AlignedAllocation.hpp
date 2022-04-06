#pragma once

#include <cstdint>

namespace Emergence::Memory::Original
{
// We don't use const below, because going through a lot of consts with pointers is painful.

void *AlignedAllocate (size_t _alignment, size_t _amount) noexcept;

void *AlignedReallocate (void *_block, size_t _alignment, size_t _oldSize, size_t _newSize) noexcept;

void AlignedFree (void *_block) noexcept;

size_t CorrectPageChunkSize (size_t _alignment, size_t _requestedChunkSize) noexcept;

size_t GetPageSize (size_t _chunkSize, size_t _capacity) noexcept;

using AlignedPoolPage = void;

void *GetPageChunksBegin (AlignedPoolPage *_page) noexcept;

void *GetPageChunksEnd (AlignedPoolPage *_page, size_t _chunkSize, size_t _capacity) noexcept;

AlignedPoolPage *&NextPagePointer (AlignedPoolPage *_page, size_t _chunkSize, size_t _capacity) noexcept;

constexpr size_t GetPageMetadataSize () noexcept
{
    return sizeof (uintptr_t);
}
} // namespace Emergence::Memory::Original

#define _CRT_SECURE_NO_WARNINGS

#include <cstdlib>
#include <cstring>

#include <Memory/Original/AlignedAllocation.hpp>

namespace Emergence::Memory::Original
{
void *AlignedAllocate (size_t _alignment, size_t _amount) noexcept
{
#if defined(_MSVC_STL_VERSION)
    return _aligned_malloc (CorrectAlignedBlockSize (_alignment, _amount), _alignment);
#else
    return std::aligned_alloc (_alignment, _amount);
#endif
}

void *AlignedReallocate (void *_block, size_t _alignment, [[maybe_unused]] size_t _oldSize, size_t _newSize) noexcept
{
#if defined(_MSVC_STL_VERSION)
    return _aligned_realloc (_block, CorrectAlignedBlockSize (_alignment, _newSize), _alignment);
#else
    // Aligned realloc is not supported by STL, but it is needed for some rare cases.
    // This implementation is not the best, because it does allocate+copy every time.
    // But it is not possible to design better implementation without under-the-hood access to the allocation data.

    void *newStorage = AlignedAllocate (_alignment, _newSize);
    memcpy (newStorage, _block, _oldSize);
    AlignedFree (_block);
    return newStorage;
#endif
}

void AlignedFree (void *_block) noexcept
{
#if defined(_MSVC_STL_VERSION)
    return _aligned_free (_block);
#else
    return std::free (_block);
#endif
}

size_t CorrectAlignedBlockSize (size_t _alignment, size_t _requestedBlockSize) noexcept
{
    if (const size_t leftover = _requestedBlockSize % _alignment)
    {
        return _requestedBlockSize + _alignment - leftover;
    }

    return _requestedBlockSize;
}

size_t GetPageSize (size_t _chunkSize, size_t _capacity) noexcept
{
    return _chunkSize * _capacity + GetPageMetadataSize ();
}

void *GetPageChunksBegin (AlignedPoolPage *_page) noexcept
{
    return _page;
}

void *GetPageChunksEnd (AlignedPoolPage *_page, size_t _chunkSize, size_t _capacity) noexcept
{
    return static_cast<uint8_t *> (_page) + _chunkSize * _capacity;
}

AlignedPoolPage *&NextPagePointer (AlignedPoolPage *_page, size_t _chunkSize, size_t _capacity) noexcept
{
    return *reinterpret_cast<AlignedPoolPage **> (GetPageChunksEnd (_page, _chunkSize, _capacity));
}
} // namespace Emergence::Memory::Original

#include <cstdlib>

#include <Assert/Assert.hpp>

#include <Memory/Original/UnorderedPool.hpp>

namespace Emergence::Memory::Original
{
UnorderedPool::UnorderedPool (Profiler::AllocationGroup _group,
                              size_t _chunkSize,
                              size_t _alignment,
                              size_t _pageCapacity) noexcept
    : chunkSize (CorrectAlignedBlockSize (_alignment, _chunkSize)),
      alignment (_alignment),
      pageCapacity (_pageCapacity),
      group (std::move (_group))
{
    EMERGENCE_ASSERT (_pageCapacity > 0u);
    EMERGENCE_ASSERT (_chunkSize >= sizeof (Chunk));
}

UnorderedPool::UnorderedPool (UnorderedPool &&_other) noexcept
    : chunkSize (_other.chunkSize),
      alignment (_other.alignment),
      pageCapacity (_other.pageCapacity),
      topPage (_other.topPage),
      topFreeChunk (_other.topFreeChunk),
      acquiredChunkCount (_other.acquiredChunkCount),
      group (std::move (_other.group))
{
    _other.topPage = nullptr;
    _other.topFreeChunk = nullptr;
    _other.acquiredChunkCount = 0u;
}

UnorderedPool::~UnorderedPool () noexcept
{
    Clear ();
}

void *UnorderedPool::Acquire () noexcept
{
    if (!topFreeChunk)
    {
        const size_t pageSize = GetPageSize (chunkSize, pageCapacity);
        group.Allocate (pageSize);
        group.Acquire (GetPageMetadataSize ());

        auto *newPage = static_cast<AlignedPoolPage *> (AlignedAllocate (alignment, pageSize));
        SetNextPagePointer (newPage, chunkSize, pageCapacity, topPage);
        topPage = newPage;
        auto *currentChunk = static_cast<Chunk *> (GetPageChunksBegin (newPage));

        for (size_t nextChunkIndex = 1u; nextChunkIndex < pageCapacity; ++nextChunkIndex)
        {
            auto *next = reinterpret_cast<Chunk *> (reinterpret_cast<uint8_t *> (currentChunk) + chunkSize);
            currentChunk->nextFree = next;
            currentChunk = next;
        }

        currentChunk->nextFree = nullptr;
        topFreeChunk = static_cast<Chunk *> (GetPageChunksBegin (newPage));
    }

    group.Acquire (chunkSize);
    Chunk *acquired = topFreeChunk;
    topFreeChunk = acquired->nextFree;
    ++acquiredChunkCount;
    return acquired;
}

void UnorderedPool::Release (void *_chunk) noexcept
{
    group.Release (chunkSize);
    auto *chunk = static_cast<Chunk *> (_chunk);
    chunk->nextFree = topFreeChunk;
    topFreeChunk = chunk;

    EMERGENCE_ASSERT (acquiredChunkCount > 0u);
    --acquiredChunkCount;
}

void UnorderedPool::Clear () noexcept
{
    group.Release (chunkSize * acquiredChunkCount);
    acquiredChunkCount = 0u;

    AlignedPoolPage *page = topPage;
    const size_t pageSize = GetPageSize (chunkSize, pageCapacity);

    while (page)
    {
        group.Release (GetPageMetadataSize ());
        group.Free (pageSize);

        AlignedPoolPage *next = GetNextPagePointer (page, chunkSize, pageCapacity);
        AlignedFree (page);
        page = next;
    }

    topPage = nullptr;
    topFreeChunk = nullptr;
}

bool UnorderedPool::IsEmpty () const noexcept
{
    return acquiredChunkCount == 0u;
}

const Profiler::AllocationGroup &UnorderedPool::GetAllocationGroup () const noexcept
{
    return group;
}
} // namespace Emergence::Memory::Original

#include <cassert>
#include <cstdlib>

#include <Memory/Original/UnorderedPool.hpp>

namespace Emergence::Memory::Original
{
UnorderedPool::UnorderedPool (Profiler::AllocationGroup _group, size_t _chunkSize, size_t _pageCapacity) noexcept
    : pageCapacity (_pageCapacity),
      chunkSize (_chunkSize),
      topPage (nullptr),
      topFreeChunk (nullptr),
      acquiredChunkCount (0u),
      group (std::move (_group))
{
    assert (_pageCapacity > 0u);
    assert (_chunkSize >= sizeof (Chunk));
}

UnorderedPool::UnorderedPool (UnorderedPool &&_other) noexcept
    : pageCapacity (_other.pageCapacity),
      chunkSize (_other.chunkSize),
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
        group.Allocate (sizeof (Page) + pageCapacity * chunkSize);
        group.Acquire (sizeof (Page));

        Page *newPage = static_cast<Page *> (malloc (sizeof (Page) + pageCapacity * chunkSize));
        newPage->next = topPage;
        topPage = newPage;
        Chunk *currentChunk = &newPage->chunks[0u];

        for (size_t nextChunkIndex = 1u; nextChunkIndex < pageCapacity; ++nextChunkIndex)
        {
            auto *next = reinterpret_cast<Chunk *> (reinterpret_cast<uint8_t *> (currentChunk) + chunkSize);
            currentChunk->nextFree = next;
            currentChunk = next;
        }

        currentChunk->nextFree = nullptr;
        topFreeChunk = &newPage->chunks[0u];
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

    assert (acquiredChunkCount > 0u);
    --acquiredChunkCount;
}

void UnorderedPool::Clear () noexcept
{
    group.Release (chunkSize * acquiredChunkCount);
    acquiredChunkCount = 0u;
    Page *page = topPage;

    while (page)
    {
        group.Release (sizeof (Page));
        group.Free (sizeof (Page) + pageCapacity * chunkSize);

        Page *next = page->next;
        free (page);
        page = next;
    }

    topPage = nullptr;
    topFreeChunk = nullptr;
}

const Profiler::AllocationGroup &UnorderedPool::GetAllocationGroup () const noexcept
{
    return group;
}
} // namespace Emergence::Memory::Original

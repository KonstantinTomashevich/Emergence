#include <cassert>
#include <cstdlib>

#include <Memory/Original/UnorderedPool.hpp>

namespace Emergence::Memory::Original
{
UnorderedPool::UnorderedPool (size_t _chunkSize, size_t _pageCapacity) noexcept
    : pageCapacity (_pageCapacity),
      chunkSize (_chunkSize),
      pageCount (0u),
      topPage (nullptr),
      topFreeChunk (nullptr)
{
    assert (_pageCapacity > 0u);
    assert (_chunkSize >= sizeof (Chunk));
}

UnorderedPool::UnorderedPool (UnorderedPool &&_other) noexcept
    : pageCapacity (_other.pageCapacity),
      chunkSize (_other.chunkSize),
      pageCount (_other.pageCount),
      topPage (_other.topPage),
      topFreeChunk (_other.topFreeChunk)
{
    _other.pageCount = 0u;
    _other.topPage = nullptr;
    _other.topFreeChunk = nullptr;
}

UnorderedPool::~UnorderedPool () noexcept
{
    Clear ();
}

void *UnorderedPool::Acquire () noexcept
{
    if (!topFreeChunk)
    {
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
        assert (pageCount + 1u > pageCount);
        ++pageCount;
    }

    Chunk *acquired = topFreeChunk;
    topFreeChunk = acquired->nextFree;
    return acquired;
}

void UnorderedPool::Release (void *_chunk) noexcept
{
    auto *chunk = static_cast<Chunk *> (_chunk);
    chunk->nextFree = topFreeChunk;
    topFreeChunk = chunk;
}

void UnorderedPool::Clear () noexcept
{
    Page *page = topPage;
    while (page)
    {
        Page *next = page->next;
        free (page);
        page = next;
    }

    pageCount = 0u;
    topPage = nullptr;
    topFreeChunk = nullptr;
}

size_t UnorderedPool::GetAllocatedSpace () const noexcept
{
    return pageCount * pageCapacity * chunkSize;
}
} // namespace Emergence::Memory::Original

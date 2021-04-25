#include <cassert>
#include <cstdlib>
#include <vector>

#include <Memory/Pool/UnorderedPool.hpp>

namespace Emergence::Memory
{
UnorderedPool::UnorderedPool (size_t _chunkSize, size_t _pageCapacity)
    : chunkSize (_chunkSize),
      pageCapacity (_pageCapacity),
      topPage (nullptr),
      topFreeChunk (nullptr)
{
    assert (_pageCapacity > 0u);
    assert (_chunkSize >= sizeof (Chunk));
}

UnorderedPool::UnorderedPool (UnorderedPool &&_other)
    : chunkSize (_other.chunkSize),
      pageCapacity (_other.pageCapacity),
      topPage (_other.topPage),
      topFreeChunk (_other.topFreeChunk)
{
    _other.topPage = nullptr;
    _other.topFreeChunk = nullptr;
}

UnorderedPool::~UnorderedPool ()
{
    Clear ();
}

void *UnorderedPool::Acquire () noexcept
{
    if (!topFreeChunk)
    {
        Page *newPage = static_cast <Page *> (malloc (sizeof (Page) + pageCapacity * chunkSize));
        newPage->next = topPage;
        topPage = newPage;
        Chunk *currentChunk = GetFirstChunk (newPage);

        for (size_t index = 1u; index < pageCapacity; ++index)
        {
            auto *next = reinterpret_cast <Chunk *> (reinterpret_cast <uint8_t *> (currentChunk) + chunkSize);
            currentChunk->nextFree = next;
            currentChunk = next;
        }

        currentChunk->nextFree->nextFree = nullptr;
        topFreeChunk = GetFirstChunk (newPage);
    }

    Chunk *acquired = topFreeChunk;
    topFreeChunk = acquired->nextFree;
    return acquired;
}

void UnorderedPool::Release (void *_chunk) noexcept
{
    auto chunk = static_cast <Chunk *> (_chunk);

#ifndef NDEBUG
    {
        bool inside = false;
        Page *page = topPage;

        while (page)
        {
            if (IsInside (page, chunk))
            {
                inside = true;
                break;
            }

            page = page->next;
        }

        assert (inside);
    }
#endif

    chunk->nextFree = topFreeChunk;
    topFreeChunk = chunk;
}

void UnorderedPool::Shrink () noexcept
{
    if (!topPage || !topFreeChunk)
    {
        return;
    }

    size_t pageCount = 0u;
    Page *currentPage = topPage;

    while (currentPage)
    {
        ++pageCount;
        currentPage = currentPage->next;
    }

    std::vector <size_t> freeChunkCounters (pageCount, 0u);
    Chunk *currentFreeChunk = topFreeChunk;

    while (currentFreeChunk)
    {
        currentPage = topPage;
        size_t pageIndex = 0u;

        while (currentPage)
        {
            if (IsInside (currentPage, currentFreeChunk))
            {
                ++freeChunkCounters[pageIndex];
                break;
            }

            ++pageIndex;
            currentPage = currentPage->next;
        }

        currentFreeChunk = currentFreeChunk->nextFree;
    }

    size_t currentPageIndex = 0u;
    Page *previousPage = nullptr;
    currentPage = topPage;

    while (currentPage)
    {
        Page *nextPage = currentPage->next;
        if (freeChunkCounters[currentPageIndex] == pageCapacity)
        {
            free (currentPage);
            if (previousPage)
            {
                previousPage->next = nextPage;
            }
            else
            {
                topPage = nextPage;
            }
        }
        else
        {
            previousPage = currentPage;
        }

        ++currentPageIndex;
        currentPage = nextPage;
    }
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
}

UnorderedPool::Chunk *UnorderedPool::GetFirstChunk (UnorderedPool::Page *_page)
{
    return const_cast <Chunk *> (GetFirstChunk (const_cast <const Page *> (_page)));
}

const UnorderedPool::Chunk *UnorderedPool::GetFirstChunk (const UnorderedPool::Page *_page) const
{
    return reinterpret_cast <const Chunk *> (_page + 1u);
}

bool UnorderedPool::IsInside (const UnorderedPool::Page *_page, const UnorderedPool::Chunk *_chunk) const
{
    const Chunk *first = GetFirstChunk (_page);
    const auto *last = reinterpret_cast <const Chunk *> (
        reinterpret_cast <const uint8_t *> (first) + chunkSize * (pageCapacity - 1u));

    return _chunk >= first && _chunk <= last;
}
} // namespace Emergence::Memory
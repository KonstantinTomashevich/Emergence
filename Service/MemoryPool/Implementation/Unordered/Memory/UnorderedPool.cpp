#include <cassert>
#include <cstdlib>
#include <vector>

#include <Memory/UnorderedPool.hpp>

namespace Emergence::Memory
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
        Page *newPage = static_cast <Page *> (malloc (sizeof (Page) + pageCapacity * chunkSize));
        newPage->next = topPage;
        topPage = newPage;
        Chunk *currentChunk = GetFirstChunk (newPage);

        for (size_t nextChunkIndex = 1u; nextChunkIndex < pageCapacity; ++nextChunkIndex)
        {
            auto *next = reinterpret_cast <Chunk *> (reinterpret_cast <uint8_t *> (currentChunk) + chunkSize);
            currentChunk->nextFree = next;
            currentChunk = next;
        }

        currentChunk->nextFree = nullptr;
        topFreeChunk = GetFirstChunk (newPage);
        assert (pageCount + 1u > pageCount);
        ++pageCount;
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

    // Pool Shrink operation is always slow (especially for unordered pools),
    // therefore it's not so bad to dynamically create array here.
    std::vector <size_t> freeChunkCounters (pageCount, 0u);
    Chunk *currentFreeChunk = topFreeChunk;

    while (currentFreeChunk)
    {
        Page *currentPage = topPage;
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
    Page *currentPage = topPage;

    while (currentPage)
    {
        Page *nextPage = currentPage->next;
        if (freeChunkCounters[currentPageIndex] == pageCapacity)
        {
            free (currentPage);
            assert (pageCount - 1u < pageCount);
            --pageCount;

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

    pageCount = 0u;
    topPage = nullptr;
    topFreeChunk = nullptr;
}

size_t UnorderedPool::GetAllocatedSpace () const noexcept
{
    return pageCount * pageCapacity * chunkSize;
}

UnorderedPool::Chunk *UnorderedPool::GetFirstChunk (UnorderedPool::Page *_page) noexcept
{
    return const_cast <Chunk *> (GetFirstChunk (const_cast <const Page *> (_page)));
}

const UnorderedPool::Chunk *UnorderedPool::GetFirstChunk (const UnorderedPool::Page *_page) const noexcept
{
    return reinterpret_cast <const Chunk *> (_page + 1u);
}

bool UnorderedPool::IsInside (const UnorderedPool::Page *_page, const UnorderedPool::Chunk *_chunk) const noexcept
{
    const Chunk *first = GetFirstChunk (_page);
    const auto *last = reinterpret_cast <const Chunk *> (
        reinterpret_cast <const uint8_t *> (first) + chunkSize * (pageCapacity - 1u));

    return _chunk >= first && _chunk <= last;
}
} // namespace Emergence::Memory
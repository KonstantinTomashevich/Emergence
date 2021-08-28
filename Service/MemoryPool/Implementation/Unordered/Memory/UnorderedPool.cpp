#include <cassert>
#include <cstdlib>
#include <vector>

#include <Memory/UnorderedPool.hpp>

namespace Emergence::Memory
{
const void *UnorderedPool::AcquiredChunkConstIterator::operator* () const noexcept
{
    assert (page);
    assert (chunk);
    return chunk;
}

UnorderedPool::AcquiredChunkConstIterator &UnorderedPool::AcquiredChunkConstIterator::operator++ () noexcept
{
    assert (pool);
    assert (page);
    assert (chunk);

    do
    {
        const auto *next = reinterpret_cast<const Chunk *> (&chunk->bytes[0u] + pool->chunkSize);
        if (&next->bytes[0u] < &page->chunks[0u].bytes[0u] + pool->chunkSize * pool->pageCapacity)
        {
            chunk = next;
        }
        else
        {
            page = page->next;
            if (page)
            {
                chunk = &page->chunks[0u];
            }
            else
            {
                chunk = nullptr;
                break;
            }
        }
    } while (pool->IsFree (chunk));

    return *this;
}

UnorderedPool::AcquiredChunkConstIterator UnorderedPool::AcquiredChunkConstIterator::operator++ (int) noexcept
{
    AcquiredChunkConstIterator previousValue (pool, page, chunk);
    ++*this;
    return previousValue;
}

UnorderedPool::AcquiredChunkConstIterator::AcquiredChunkConstIterator (const UnorderedPool *_pool,
                                                                       const Page *_page) noexcept
    : AcquiredChunkConstIterator (_pool, _page, _page ? &_page->chunks[0u] : nullptr)
{
    if (chunk)
    {
        // If first chunk of the top page is already free, use increment to find first acquired chunk.
        if (pool->IsFree (chunk))
        {
            ++*this;
        }
    }
}

UnorderedPool::AcquiredChunkConstIterator::AcquiredChunkConstIterator (const UnorderedPool *_pool,
                                                                       const Page *_page,
                                                                       const Chunk *_chunk) noexcept
    : pool (_pool),
      page (_page),
      chunk (_chunk)
{
    assert (pool);
#ifndef NDEBUG
    if (page)
    {
        assert (chunk);
        assert (pool->IsInside (page, chunk));
    }
    else
    {
        assert (!chunk);
    }
#endif
}

void *UnorderedPool::AcquiredChunkIterator::operator* () const noexcept
{
    return const_cast<void *> (*coreIterator);
}

UnorderedPool::AcquiredChunkIterator &UnorderedPool::AcquiredChunkIterator::operator++ () noexcept
{
    ++coreIterator;
    return *this;
}

UnorderedPool::AcquiredChunkIterator UnorderedPool::AcquiredChunkIterator::operator++ (int) noexcept
{
    AcquiredChunkIterator previousValue (coreIterator);
    ++*this;
    return previousValue;
}

UnorderedPool::AcquiredChunkIterator::AcquiredChunkIterator (const AcquiredChunkConstIterator &_coreIterator) noexcept
    : coreIterator (_coreIterator)
{
}

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
    auto chunk = static_cast<Chunk *> (_chunk);

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
    std::vector<size_t> freeChunkCounters (pageCount, 0u);
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

bool UnorderedPool::IsInside (const UnorderedPool::Page *_page, const UnorderedPool::Chunk *_chunk) const noexcept
{
    const Chunk *first = &_page->chunks[0u];
    const auto *last = reinterpret_cast<const Chunk *> (&first->bytes[0u] + chunkSize * (pageCapacity - 1u));
    return _chunk >= first && _chunk <= last;
}

bool UnorderedPool::IsFree (const UnorderedPool::Chunk *_chunk) const noexcept
{
    const Chunk *currentFreeChunk = topFreeChunk;
    while (currentFreeChunk)
    {
        if (currentFreeChunk == _chunk)
        {
            return true;
        }

        currentFreeChunk = currentFreeChunk->nextFree;
    }

    return false;
}

UnorderedPool::AcquiredChunkConstIterator UnorderedPool::BeginAcquired () const noexcept
{
    return AcquiredChunkConstIterator (this, topPage);
}

UnorderedPool::AcquiredChunkConstIterator UnorderedPool::EndAcquired () const noexcept
{
    return AcquiredChunkConstIterator (this, nullptr);
}

UnorderedPool::AcquiredChunkIterator UnorderedPool::BeginAcquired () noexcept
{
    return AcquiredChunkIterator (static_cast<const UnorderedPool *> (this)->BeginAcquired ());
}

UnorderedPool::AcquiredChunkIterator UnorderedPool::EndAcquired () noexcept
{
    return AcquiredChunkIterator (static_cast<const UnorderedPool *> (this)->EndAcquired ());
}
} // namespace Emergence::Memory

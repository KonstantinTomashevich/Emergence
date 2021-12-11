#include <cassert>
#include <cstdlib>

#include <Memory/Original/OrderedPool.hpp>
#include <utility>

namespace Emergence::Memory::Original
{
OrderedPool::AcquiredChunkConstIterator::AcquiredChunkConstIterator (
    const AcquiredChunkConstIterator &_other) noexcept = default;

OrderedPool::AcquiredChunkConstIterator::AcquiredChunkConstIterator (AcquiredChunkConstIterator &&_other) noexcept =
    default;

const void *OrderedPool::AcquiredChunkConstIterator::operator* () const noexcept
{
    return currentChunk;
}

OrderedPool::AcquiredChunkConstIterator &OrderedPool::AcquiredChunkConstIterator::operator= (
    const AcquiredChunkConstIterator &_other) noexcept = default;

OrderedPool::AcquiredChunkConstIterator &OrderedPool::AcquiredChunkConstIterator::operator= (
    AcquiredChunkConstIterator &&_other) noexcept = default;

OrderedPool::AcquiredChunkConstIterator::~AcquiredChunkConstIterator () noexcept = default;

OrderedPool::AcquiredChunkConstIterator &OrderedPool::AcquiredChunkConstIterator::operator++ () noexcept
{
    assert (currentPage);
    assert (currentChunk);

    const std::size_t pageSize = pool->chunkSize * pool->pageCapacity;
    const auto *pageEnd =
        reinterpret_cast<const Chunk *> (reinterpret_cast<const uint8_t *> (&currentPage->chunks[0u]) + pageSize);

    while (true)
    {
        currentChunk = reinterpret_cast<const Chunk *> (&currentChunk->bytes[0u] + pool->chunkSize);
        if (currentChunk >= pageEnd)
        {
            currentPage = currentPage->next;
            if (currentPage)
            {
                pageEnd = reinterpret_cast<const Chunk *> (
                    reinterpret_cast<const uint8_t *> (&currentPage->chunks[0u]) + pageSize);
                currentChunk = &currentPage->chunks[0u];
            }
            else
            {
                // No more pages: iteration finished.
                currentChunk = nullptr;
                break;
            }
        }

        while (currentFreeChunk && currentFreeChunk < currentChunk)
        {
            currentFreeChunk = currentFreeChunk->nextFree;
        }

        if (currentFreeChunk != currentChunk)
        {
            // Chunk is acquired.
            break;
        }
    }

    return *this;
}

OrderedPool::AcquiredChunkConstIterator OrderedPool::AcquiredChunkConstIterator::operator++ (int) noexcept
{
    AcquiredChunkConstIterator previous = *this;
    ++*this;
    return previous;
}

bool OrderedPool::AcquiredChunkConstIterator::operator== (const AcquiredChunkConstIterator &_other) const noexcept
{
    return currentChunk == _other.currentChunk;
}

bool OrderedPool::AcquiredChunkConstIterator::operator!= (const AcquiredChunkConstIterator &_other) const noexcept
{
    return !(*this == _other);
}

OrderedPool::AcquiredChunkConstIterator::AcquiredChunkConstIterator (const OrderedPool *_pool) noexcept
    : currentPage (_pool->topPage),
      currentFreeChunk (_pool->topFreeChunk),
      pool (_pool)
{
    if (pool->topPage)
    {
        currentChunk = &currentPage->chunks[0u];

        // Because ordering is guaranteed, we can check is first chunk free by this simple comparison.
        if (currentChunk == currentFreeChunk)
        {
            ++*this;
        }
    }
}

OrderedPool::AcquiredChunkIterator::AcquiredChunkIterator (const AcquiredChunkIterator &_other) noexcept = default;

OrderedPool::AcquiredChunkIterator::AcquiredChunkIterator (AcquiredChunkIterator &&_other) noexcept = default;

OrderedPool::AcquiredChunkIterator::~AcquiredChunkIterator () noexcept = default;

void *OrderedPool::AcquiredChunkIterator::operator* () const noexcept
{
    return const_cast<void *> (*base);
}

OrderedPool::AcquiredChunkIterator &OrderedPool::AcquiredChunkIterator::operator++ () noexcept
{
    ++base;
    return *this;
}

OrderedPool::AcquiredChunkIterator OrderedPool::AcquiredChunkIterator::operator++ (int) noexcept
{
    AcquiredChunkIterator previous = *this;
    ++*this;
    return previous;
}

OrderedPool::AcquiredChunkIterator &OrderedPool::AcquiredChunkIterator::operator= (
    const AcquiredChunkIterator &_other) noexcept = default;

OrderedPool::AcquiredChunkIterator &OrderedPool::AcquiredChunkIterator::operator= (
    AcquiredChunkIterator &&_other) noexcept = default;

bool OrderedPool::AcquiredChunkIterator::operator== (const AcquiredChunkIterator &_other) const noexcept
{
    return base == _other.base;
}

bool OrderedPool::AcquiredChunkIterator::operator!= (const AcquiredChunkIterator &_other) const noexcept
{
    return !(*this == _other);
}

OrderedPool::AcquiredChunkIterator::AcquiredChunkIterator (AcquiredChunkConstIterator _base) noexcept
    : base (std::move (_base))
{
}

OrderedPool::OrderedPool (size_t _chunkSize, size_t _pageCapacity) noexcept
    : pageCapacity (_pageCapacity),
      chunkSize (_chunkSize),
      pageCount (0u),
      topPage (nullptr),
      topFreeChunk (nullptr)
{
    assert (_pageCapacity > 0u);
    assert (_chunkSize >= sizeof (Chunk));
}

OrderedPool::OrderedPool (OrderedPool &&_other) noexcept
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

OrderedPool::~OrderedPool () noexcept
{
    Clear ();
}

void *OrderedPool::Acquire () noexcept
{
    if (!topFreeChunk)
    {
        Page *newPage = static_cast<Page *> (malloc (sizeof (Page) + pageCapacity * chunkSize));
        Page *insertPageBefore = topPage;
        Page *insertPageAfter = nullptr;

        while (insertPageBefore && newPage > insertPageBefore)
        {
            insertPageAfter = insertPageBefore;
            insertPageBefore = insertPageBefore->next;
        }

        newPage->next = insertPageBefore;
        if (insertPageAfter)
        {
            insertPageAfter->next = newPage;
        }
        else
        {
            topPage = newPage;
        }

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

void OrderedPool::Release (void *_chunk) noexcept
{
    auto *chunk = static_cast<Chunk *> (_chunk);
    Chunk *insertChunkAfter = nullptr;
    Chunk *insertChunkBefore = topFreeChunk;

    while (insertChunkBefore && chunk > insertChunkBefore)
    {
        insertChunkAfter = insertChunkBefore;
        insertChunkBefore = insertChunkBefore->nextFree;
    }

    chunk->nextFree = insertChunkBefore;
    if (insertChunkAfter)
    {
        insertChunkAfter->nextFree = chunk;
    }
    else
    {
        topFreeChunk = chunk;
    }
}

void OrderedPool::Shrink () noexcept
{
    Page *previousPage = nullptr;
    Page *currentPage = topPage;
    Chunk *currentFreeChunk = topFreeChunk;

    while (currentPage)
    {
        while (currentFreeChunk < &currentPage->chunks[0u])
        {
            currentFreeChunk = currentFreeChunk->nextFree;

            // All pages from this and above are fully filled, therefore shrink is finished.
            if (!currentFreeChunk)
            {
                return;
            }
        }

        // Count free chunks that belong to this page.
        std::size_t pageFreeChunks = 0u;
        auto *pageEnd = reinterpret_cast<Chunk *> (reinterpret_cast<uint8_t *> (&currentPage->chunks[0u]) +
                                                   chunkSize * pageCapacity);

        while (currentFreeChunk && currentFreeChunk < pageEnd)
        {
            ++pageFreeChunks;
            currentFreeChunk = currentFreeChunk->nextFree;
        }

        if (pageFreeChunks == pageCapacity)
        {
            // All chunks in current page are free, therefore we can safely release it.
            Page *nextPage = currentPage->next;
            free (currentPage);
            currentPage = nextPage;
            --pageCount;

            if (previousPage)
            {
                previousPage->next = currentPage;
            }
            else
            {
                topPage = currentPage;
            }
        }
        else
        {
            previousPage = currentPage;
            currentPage = currentPage->next;
        }
    }
}

void OrderedPool::Clear () noexcept
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

OrderedPool::AcquiredChunkConstIterator OrderedPool::BeginAcquired () const noexcept
{
    return AcquiredChunkConstIterator {this};
}
OrderedPool::AcquiredChunkConstIterator OrderedPool::EndAcquired () const noexcept
{
    return AcquiredChunkConstIterator {};
}

OrderedPool::AcquiredChunkIterator OrderedPool::BeginAcquired () noexcept
{
    return AcquiredChunkIterator {const_cast<const OrderedPool *> (this)->BeginAcquired ()};
}
OrderedPool::AcquiredChunkIterator OrderedPool::EndAcquired () noexcept
{
    return AcquiredChunkIterator {const_cast<const OrderedPool *> (this)->EndAcquired ()};
}

size_t OrderedPool::GetAllocatedSpace () const noexcept
{
    return pageCount * pageCapacity * chunkSize;
}
} // namespace Emergence::Memory::Original

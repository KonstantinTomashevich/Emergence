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

OrderedPool::OrderedPool (Profiler::AllocationGroup _group, size_t _chunkSize, size_t _pageCapacity) noexcept
    : pageCapacity (_pageCapacity),
      chunkSize (_chunkSize),
      topPage (nullptr),
      topFreeChunk (nullptr),
      group (std::move(_group))
{
    assert (_pageCapacity > 0u);
    assert (_chunkSize >= sizeof (Chunk));
}

OrderedPool::OrderedPool (OrderedPool &&_other) noexcept
    : pageCapacity (_other.pageCapacity),
      chunkSize (_other.chunkSize),
      topPage (_other.topPage),
      topFreeChunk (_other.topFreeChunk),
      group (std::move (_other.group))
{
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
        group.Allocate (sizeof (Page) + pageCapacity * chunkSize);
        group.Acquire (sizeof (Page));

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
    }

    group.Acquire (chunkSize);
    Chunk *acquired = topFreeChunk;
    topFreeChunk = acquired->nextFree;
    return acquired;
}

void OrderedPool::Release (void *_chunk) noexcept
{
    group.Release (chunkSize);
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

    while (currentPage && currentFreeChunk)
    {
        // Due to the ordering and shrink logic, this assert should always be true.
        // It's left here to highlight that this case is impossible in current algorithm.
        assert (currentFreeChunk >= &currentPage->chunks[0u]);

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

            group.Release (sizeof (Page));
            group.Free (sizeof (Page) + pageCapacity * chunkSize);

            currentPage = nextPage;
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
        group.Release (sizeof (Page));
        group.Free (sizeof (Page) + pageCapacity * chunkSize);

        Page *next = page->next;
        free (page);
        page = next;
    }

    topPage = nullptr;
    topFreeChunk = nullptr;
}

OrderedPool::AcquiredChunkConstIterator OrderedPool::BeginAcquired () const noexcept
{
    return AcquiredChunkConstIterator {this};
}

// NOLINTNEXTLINE(readability-convert-member-functions-to-static): It's more readable to keep End* methods as members.
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

const Profiler::AllocationGroup &OrderedPool::GetAllocationGroup () const noexcept
{
    return group;
}
} // namespace Emergence::Memory::Original

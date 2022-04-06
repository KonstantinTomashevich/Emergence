#include <cassert>
#include <cstdlib>

#include <Memory/Original/OrderedPool.hpp>

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

    const void *pageEnd =
        GetPageChunksEnd (const_cast<AlignedPoolPage *> (currentPage), pool->chunkSize, pool->pageCapacity);

    while (true)
    {
        currentChunk = reinterpret_cast<const Chunk *> (&currentChunk->bytes[0u] + pool->chunkSize);
        if (currentChunk >= pageEnd)
        {
            currentPage =
                NextPagePointer (const_cast<AlignedPoolPage *> (currentPage), pool->chunkSize, pool->pageCapacity);

            if (currentPage)
            {
                pageEnd =
                    GetPageChunksEnd (const_cast<AlignedPoolPage *> (currentPage), pool->chunkSize, pool->pageCapacity);

                currentChunk = static_cast<Chunk *> (GetPageChunksBegin (const_cast<AlignedPoolPage *> (currentPage)));
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
    if (currentPage)
    {
        currentChunk = static_cast<Chunk *> (GetPageChunksBegin (const_cast<AlignedPoolPage *> (currentPage)));

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

OrderedPool::OrderedPool (Profiler::AllocationGroup _group,
                          size_t _chunkSize,
                          size_t _alignment,
                          size_t _pageCapacity) noexcept
    : chunkSize (CorrectAlignedBlockSize (_alignment, _chunkSize)),
      alignment (_alignment),
      pageCapacity (_pageCapacity),
      topPage (nullptr),
      topFreeChunk (nullptr),
      acquiredChunkCount (0u),
      group (std::move (_group))
{
    assert (_pageCapacity > 0u);
    assert (_chunkSize >= sizeof (Chunk));
}

OrderedPool::OrderedPool (OrderedPool &&_other) noexcept
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

OrderedPool::~OrderedPool () noexcept
{
    Clear ();
}

void *OrderedPool::Acquire () noexcept
{
    if (!topFreeChunk)
    {
        const size_t pageSize = GetPageSize (chunkSize, pageCapacity);
        group.Allocate (pageSize);
        group.Acquire (GetPageMetadataSize ());

        auto *newPage = static_cast<AlignedPoolPage *> (AlignedAllocate (alignment, pageSize));
        AlignedPoolPage *insertPageBefore = topPage;
        AlignedPoolPage *insertPageAfter = nullptr;

        while (insertPageBefore && newPage > insertPageBefore)
        {
            insertPageAfter = insertPageBefore;
            insertPageBefore = NextPagePointer (insertPageBefore, chunkSize, pageCapacity);
        }

        NextPagePointer (newPage, chunkSize, pageCapacity) = insertPageBefore;
        if (insertPageAfter)
        {
            NextPagePointer (insertPageAfter, chunkSize, pageCapacity) = newPage;
        }
        else
        {
            topPage = newPage;
        }

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

    assert (acquiredChunkCount > 0u);
    --acquiredChunkCount;
}

void OrderedPool::Shrink () noexcept
{
    AlignedPoolPage *previousPage = nullptr;
    AlignedPoolPage *currentPage = topPage;
    Chunk *currentFreeChunk = topFreeChunk;
    const size_t pageSize = GetPageSize (chunkSize, pageCapacity);

    while (currentPage && currentFreeChunk)
    {
        // Due to the ordering and shrink logic, this assert should always be true.
        // It's left here to highlight that this case is impossible in current algorithm.
        assert (currentFreeChunk >= GetPageChunksBegin (currentPage));

        // Count free chunks that belong to this page.
        std::size_t pageFreeChunks = 0u;
        void *pageEnd = GetPageChunksEnd (currentPage, chunkSize, pageCapacity);

        while (currentFreeChunk && currentFreeChunk < pageEnd)
        {
            ++pageFreeChunks;
            currentFreeChunk = currentFreeChunk->nextFree;
        }

        if (pageFreeChunks == pageCapacity)
        {
            // All chunks in current page are free, therefore we can safely release it.
            AlignedPoolPage *nextPage = NextPagePointer (currentPage, chunkSize, pageCapacity);
            AlignedFree (currentPage);

            group.Release (GetPageMetadataSize ());
            group.Free (pageSize);

            currentPage = nextPage;
            if (previousPage)
            {
                NextPagePointer (previousPage, chunkSize, pageCapacity) = currentPage;
            }
            else
            {
                topPage = currentPage;
            }
        }
        else
        {
            previousPage = currentPage;
            currentPage = NextPagePointer (currentPage, chunkSize, pageCapacity);
        }
    }
}

void OrderedPool::Clear () noexcept
{
    group.Release (chunkSize * acquiredChunkCount);
    acquiredChunkCount = 0u;

    AlignedPoolPage *page = topPage;
    const size_t pageSize = GetPageSize (chunkSize, pageCapacity);

    while (page)
    {
        group.Release (GetPageMetadataSize ());
        group.Free (pageSize);

        AlignedPoolPage *next = NextPagePointer (page, chunkSize, pageCapacity);
        AlignedFree (page);
        page = next;
    }

    topPage = nullptr;
    topFreeChunk = nullptr;
}

bool OrderedPool::IsEmpty () const noexcept
{
    return acquiredChunkCount == 0u;
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

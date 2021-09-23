#pragma once

#include <cstdint>

#include <API/Common/Shortcuts.hpp>

namespace Emergence::Memory
{
class UnorderedPool final
{
private:
    struct Page;

    struct Chunk;

public:
    class AcquiredChunkConstIterator final
    {
    public:
        const void *operator* () const noexcept;

        AcquiredChunkConstIterator &operator++ () noexcept;

        AcquiredChunkConstIterator operator++ (int) noexcept;

        bool operator== (const AcquiredChunkConstIterator &_other) const noexcept = default;

        bool operator!= (const AcquiredChunkConstIterator &_other) const noexcept = default;

    private:
        /// UnorderedPool constructs iterators.
        friend class UnorderedPool;

        explicit AcquiredChunkConstIterator (const UnorderedPool *_pool, const Page *_page) noexcept;

        explicit AcquiredChunkConstIterator (const UnorderedPool *_pool,
                                             const Page *_page,
                                             const Chunk *_chunk) noexcept;

        const UnorderedPool *pool;
        const Page *page;
        const Chunk *chunk;
    };

    class AcquiredChunkIterator final
    {
    public:
        void *operator* () const noexcept;

        AcquiredChunkIterator &operator++ () noexcept;

        AcquiredChunkIterator operator++ (int) noexcept;

        bool operator== (const AcquiredChunkIterator &_other) const noexcept = default;

        bool operator!= (const AcquiredChunkIterator &_other) const noexcept = default;

    private:
        /// UnorderedPool constructs iterators.
        friend class UnorderedPool;

        explicit AcquiredChunkIterator (const AcquiredChunkConstIterator &_coreIterator) noexcept;

        AcquiredChunkConstIterator coreIterator;
    };

    UnorderedPool (size_t _chunkSize, size_t _pageCapacity) noexcept;

    UnorderedPool (const UnorderedPool &_other) = delete;

    UnorderedPool (UnorderedPool &&_other) noexcept;

    ~UnorderedPool () noexcept;

    void *Acquire () noexcept;

    void Release (void *_chunk) noexcept;

    void Shrink () noexcept;

    void Clear () noexcept;

    size_t GetAllocatedSpace () const noexcept;

    AcquiredChunkConstIterator BeginAcquired () const noexcept;

    AcquiredChunkConstIterator EndAcquired () const noexcept;

    AcquiredChunkIterator BeginAcquired () noexcept;

    AcquiredChunkIterator EndAcquired () noexcept;

    EMERGENCE_DELETE_ASSIGNMENT (UnorderedPool);

private:
    struct Chunk
    {
        union
        {
            Chunk *nextFree;

            // It should be zero-length array, but we use 1-byte array because
            // not all compilers support nested zero-length arrays.
            uint8_t bytes[1u];
        };
    };

    struct Page
    {
        Page *next;

        Chunk chunks[0u];
    };

    bool IsInside (const Page *_page, const Chunk *_chunk) const noexcept;

    bool IsFree (const Chunk *_chunk) const noexcept;

    size_t pageCapacity;
    size_t chunkSize;
    size_t pageCount;
    Page *topPage;
    Chunk *topFreeChunk;
};
} // namespace Emergence::Memory

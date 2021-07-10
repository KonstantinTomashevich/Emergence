#pragma once

#include <cstdint>

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
        ~AcquiredChunkConstIterator () noexcept = default;

        const void *operator * () const noexcept;

        AcquiredChunkConstIterator &operator ++ () noexcept;

        AcquiredChunkConstIterator operator ++ (int) noexcept;

        bool operator == (const AcquiredChunkConstIterator &_other) const noexcept = default;

        bool operator != (const AcquiredChunkConstIterator &_other) const noexcept = default;

    private:
        /// UnorderedPool constructs iterators.
        friend class UnorderedPool;

        explicit AcquiredChunkConstIterator (const UnorderedPool *_pool, const Page *_page) noexcept;

        explicit AcquiredChunkConstIterator (
            const UnorderedPool *_pool, const Page *_page, const Chunk *_chunk) noexcept;

        const UnorderedPool *pool;
        const Page *page;
        const Chunk *chunk;
    };

    class AcquiredChunkIterator final
    {
    public:
        ~AcquiredChunkIterator () noexcept = default;

        void *operator * () const noexcept;

        AcquiredChunkIterator &operator ++ () noexcept;

        AcquiredChunkIterator operator ++ (int) noexcept;

        bool operator == (const AcquiredChunkIterator &_other) const noexcept = default;

        bool operator != (const AcquiredChunkIterator &_other) const noexcept = default;

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

private:
    struct Chunk
    {
        union
        {
            Chunk *nextFree;

            // There could be more bytes. This array is used because it increases readability in algorithms.
            // It should be zero size array, but zero size arrays are not standard.
            uint8_t bytes[sizeof (void *)];
        };
    };

    struct Page
    {
        Page *next;

        // Can not be a field, because zero size arrays are not standard. Use ::GetChunks instead.
        // FieldData fields[0u];

        const Chunk *GetChunks () const noexcept;

        Chunk *GetChunks () noexcept;
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

#pragma once

#include <cstdint>

#include <API/Common/Iterator.hpp>
#include <API/Common/Shortcuts.hpp>

#include <Memory/Original/AlignedAllocation.hpp>
#include <Memory/Profiler/AllocationGroup.hpp>

namespace Emergence::Memory::Original
{
class OrderedPool final
{
private:
    struct Chunk;

public:
    class AcquiredChunkConstIterator final
    {
    public:
        EMERGENCE_FORWARD_ITERATOR_OPERATIONS (AcquiredChunkConstIterator, const void *);

    private:
        friend class OrderedPool;

        AcquiredChunkConstIterator () noexcept = default;

        AcquiredChunkConstIterator (const OrderedPool *_pool) noexcept;

        const AlignedPoolPage *currentPage = nullptr;
        const Chunk *currentChunk = nullptr;
        const Chunk *currentFreeChunk = nullptr;
        const OrderedPool *pool = nullptr;
    };

    class AcquiredChunkIterator final
    {
    public:
        EMERGENCE_FORWARD_ITERATOR_OPERATIONS (AcquiredChunkIterator, void *);

    private:
        friend class OrderedPool;

        AcquiredChunkIterator (AcquiredChunkConstIterator _base) noexcept;

        AcquiredChunkConstIterator base;
    };

    OrderedPool (Profiler::AllocationGroup _group, size_t _chunkSize, size_t _alignment, size_t _pageCapacity) noexcept;

    OrderedPool (const OrderedPool &_other) = delete;

    OrderedPool (OrderedPool &&_other) noexcept;

    ~OrderedPool () noexcept;

    void *Acquire () noexcept;

    void Release (void *_chunk) noexcept;

    void Shrink () noexcept;

    void Clear () noexcept;

    [[nodiscard]] bool IsEmpty () const noexcept;

    [[nodiscard]] AcquiredChunkConstIterator BeginAcquired () const noexcept;

    [[nodiscard]] AcquiredChunkConstIterator EndAcquired () const noexcept;

    [[nodiscard]] AcquiredChunkIterator BeginAcquired () noexcept;

    [[nodiscard]] AcquiredChunkIterator EndAcquired () noexcept;

    [[nodiscard]] const Profiler::AllocationGroup &GetAllocationGroup () const noexcept;

    EMERGENCE_DELETE_ASSIGNMENT (OrderedPool);

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

    const size_t chunkSize;
    const size_t alignment;
    const size_t pageCapacity;

    AlignedPoolPage *topPage = nullptr;
    Chunk *topFreeChunk = nullptr;

    /// \brief Acquired chunk counter required to correctly log memory usage for profiling.
    std::size_t acquiredChunkCount = 0u;

    Profiler::AllocationGroup group;
};
} // namespace Emergence::Memory::Original

#pragma once

#include <cstdint>

#include <API/Common/Shortcuts.hpp>

#include <Memory/Original/AlignedAllocation.hpp>
#include <Memory/Profiler/AllocationGroup.hpp>
#include <Memory/UniqueString.hpp>

namespace Emergence::Memory::Original
{
class UnorderedPool final
{
private:
    struct Chunk;

public:
    UnorderedPool (Profiler::AllocationGroup _group,
                   size_t _chunkSize,
                   size_t _alignment,
                   size_t _pageCapacity) noexcept;

    UnorderedPool (const UnorderedPool &_other) = delete;

    UnorderedPool (UnorderedPool &&_other) noexcept;

    ~UnorderedPool () noexcept;

    void *Acquire () noexcept;

    void Release (void *_chunk) noexcept;

    void Clear () noexcept;

    [[nodiscard]] bool IsEmpty () const noexcept;

    [[nodiscard]] const Profiler::AllocationGroup &GetAllocationGroup () const noexcept;

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

    const size_t chunkSize;
    const size_t alignment;
    const size_t pageCapacity;

    AlignedPoolPage *topPage;
    Chunk *topFreeChunk;

    /// \brief Acquired chunk counter required to correctly log memory usage for profiling.
    std::size_t acquiredChunkCount;

    Profiler::AllocationGroup group;
};
} // namespace Emergence::Memory::Original

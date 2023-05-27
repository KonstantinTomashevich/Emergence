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
                   std::size_t _chunkSize,
                   std::size_t _alignment,
                   std::size_t _pageCapacity) noexcept;

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
            std::uint8_t bytes[1u];
        };
    };

    const std::size_t chunkSize;
    const std::size_t alignment;
    const std::size_t pageCapacity;

    AlignedPoolPage *topPage = nullptr;
    Chunk *topFreeChunk = nullptr;

    /// \brief Acquired chunk counter required to correctly log memory usage for profiling.
    std::size_t acquiredChunkCount = 0u;

    Profiler::AllocationGroup group;
};
} // namespace Emergence::Memory::Original

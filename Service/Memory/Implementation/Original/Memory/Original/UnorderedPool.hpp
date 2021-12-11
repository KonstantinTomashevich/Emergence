#pragma once

#include <cstdint>

#include <API/Common/Shortcuts.hpp>

namespace Emergence::Memory::Original
{
class UnorderedPool final
{
private:
    struct Page;

    struct Chunk;

public:
    UnorderedPool (size_t _chunkSize, size_t _pageCapacity) noexcept;

    UnorderedPool (const UnorderedPool &_other) = delete;

    UnorderedPool (UnorderedPool &&_other) noexcept;

    ~UnorderedPool () noexcept;

    void *Acquire () noexcept;

    void Release (void *_chunk) noexcept;

    void Clear () noexcept;

    [[nodiscard]] size_t GetAllocatedSpace () const noexcept;

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

    const size_t pageCapacity;
    const size_t chunkSize;
    size_t pageCount;
    Page *topPage;
    Chunk *topFreeChunk;
};
} // namespace Emergence::Memory::Original

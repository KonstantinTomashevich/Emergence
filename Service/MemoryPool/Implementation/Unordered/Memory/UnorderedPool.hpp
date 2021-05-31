#pragma once

#include <cstdint>

namespace Emergence::Memory
{
class UnorderedPool final
{
public:
    UnorderedPool (size_t _chunkSize, size_t _pageCapacity) noexcept;

    UnorderedPool (const UnorderedPool &_other) = delete;

    UnorderedPool (UnorderedPool &&_other) noexcept;

    ~UnorderedPool () noexcept;

    void *Acquire () noexcept;

    void Release (void *_chunk) noexcept;

    void Shrink () noexcept;

    void Clear () noexcept;

    size_t GetAllocatedSpace () const noexcept;

private:
    struct Chunk
    {
        union
        {
            Chunk *nextFree;

            uint8_t bytes[0u];
        };
    };

    struct Page
    {
        Page *next;

        Chunk chunks[0u];
    };

    bool IsInside (const Page *_page, const Chunk *_chunk) const noexcept;

    size_t pageCapacity;
    size_t chunkSize;
    size_t pageCount;
    Page *topPage;
    Chunk *topFreeChunk;
};
} // namespace Emergence::Memory

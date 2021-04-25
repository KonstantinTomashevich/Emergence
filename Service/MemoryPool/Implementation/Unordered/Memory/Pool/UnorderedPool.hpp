#pragma once

#include <cstdint>

namespace Emergence::Memory
{
class UnorderedPool final
{
public:
    UnorderedPool (size_t _chunkSize, size_t _pageCapacity);

    UnorderedPool (const UnorderedPool &_other) = delete;

    UnorderedPool (UnorderedPool &&_other);

    ~UnorderedPool ();

    void *Acquire () noexcept;

    void Release (void *_chunk) noexcept;

    void Shrink () noexcept;

    void Clear () noexcept;

private:
    struct Chunk
    {
        union
        {
            Chunk *nextFree;

            // inplace_dynamic_array <Chunk> bytes (UnorderedPool::chunkSize);
        };
    };

    struct Page
    {
        Page *next;

        // inplace_dynamic_array <Chunk> chunks (UnorderedPool::pageCapacity);
    };

    Chunk *GetFirstChunk (Page *_page);

    const Chunk *GetFirstChunk (const Page *_page) const;

    bool IsInside (const Page *_page, const Chunk *_chunk) const;

    size_t pageCapacity;
    size_t chunkSize;
    Page *topPage;
    Chunk *topFreeChunk;
};
} // namespace Emergence::Memory

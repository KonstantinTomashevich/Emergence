#include <cassert>
#include <cstdlib>

#include <Memory/Original/Stack.hpp>

namespace Emergence::Memory::Original
{
Stack::Stack (size_t _capacity) noexcept : start (malloc (_capacity))
{
    end = static_cast<uint8_t *> (start) + _capacity;
    head = start;
}

Stack::Stack (Stack &&_other) noexcept : start (_other.start), end (_other.end), head (_other.head)
{
    _other.start = nullptr;
    _other.end = nullptr;
    _other.head = nullptr;
}

Stack::~Stack () noexcept
{
    free (start);
}

void *Stack::Acquire (size_t _chunkSize, uintptr_t _alignAs) noexcept
{
    assert (_chunkSize <= GetFreeSize ());
    if (uintptr_t alignmentStep = reinterpret_cast<uintptr_t> (head) % _alignAs)
    {
        head = static_cast<uint8_t *> (head) + _alignAs - alignmentStep;
    }

    void *allocated = head;
    head = static_cast<uint8_t *> (head) + _chunkSize;
    return allocated;
}

const void *Stack::Head () const noexcept
{
    return head;
}

void Stack::Release (const void *_newHead) noexcept
{
    assert (_newHead > start);
    assert (_newHead <= head);
    head = const_cast<void *> (_newHead);
}

void Stack::Clear () noexcept
{
    head = start;
}

size_t Stack::GetFreeSize () const noexcept
{
    return static_cast<uint8_t *> (end) - static_cast<uint8_t *> (head);
}
} // namespace Emergence::Memory::Original

#include <cassert>
#include <cstdlib>

#include <Memory/Original/Stack.hpp>

namespace Emergence::Memory::Original
{
Stack::Stack (size_t _capacity) noexcept : start (malloc (_capacity))
{
    end = static_cast<uint8_t *> (start) + _capacity;
    top = start;
}

Stack::Stack (Stack &&_other) noexcept : start (_other.start), end (_other.end), top (_other.top)
{
    _other.start = nullptr;
    _other.end = nullptr;
    _other.top = nullptr;
}

Stack::~Stack ()
{
    free (start);
}

void *Stack::Acquire (size_t _chunkSize) noexcept
{
    assert (_chunkSize <= GetFreeSize ());
    void *allocated = top;
    top = static_cast<uint8_t *> (top) + _chunkSize;
    return allocated;
}

const void *Stack::Top () const noexcept
{
    return top;
}

void Stack::Release (const void *_newTop) noexcept
{
    assert (_newTop > start);
    assert (_newTop <= top);
    top = const_cast<void *> (_newTop);
}

size_t Stack::GetFreeSize () const noexcept
{
    return static_cast<uint8_t *> (end) - static_cast<uint8_t *> (top);
}
} // namespace Emergence::Memory::Original

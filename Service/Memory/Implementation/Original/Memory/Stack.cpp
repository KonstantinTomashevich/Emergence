#include <Memory/Original/Stack.hpp>
#include <Memory/Stack.hpp>

#include <SyntaxSugar/BlockCast.hpp>

namespace Emergence::Memory
{
Stack::Stack (size_t _capacity) noexcept
{
    new (&data) Original::Stack (_capacity);
}

Stack::Stack (Stack &&_other) noexcept
{
    new (&data) Original::Stack (std::move (block_cast<Original::Stack> (_other.data)));
}

Stack::~Stack ()
{
    block_cast<Original::Stack> (data).~Stack ();
}

void *Stack::Acquire (size_t _chunkSize) noexcept
{
    return block_cast<Original::Stack> (data).Acquire (_chunkSize);
}

const void *Stack::Top () const noexcept
{
    return block_cast<Original::Stack> (data).Top ();
}

void Stack::Release (const void *_newTop) noexcept
{
    block_cast<Original::Stack> (data).Release (_newTop);
}

size_t Stack::GetFreeSize () const noexcept
{
    return block_cast<Original::Stack> (data).GetFreeSize ();
}

Stack &Stack::operator= (Stack &&_other) noexcept
{
    if (this != &_other)
    {
        this->~Stack ();
        new (this) Stack (std::move (_other));
    }

    return *this;
}
} // namespace Emergence::Memory

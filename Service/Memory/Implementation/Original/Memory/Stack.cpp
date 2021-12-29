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

Stack::~Stack () noexcept
{
    block_cast<Original::Stack> (data).~Stack ();
}

void *Stack::Acquire (size_t _chunkSize, uintptr_t _alignAs) noexcept
{
    return block_cast<Original::Stack> (data).Acquire (_chunkSize, _alignAs);
}

const void *Stack::Head () const noexcept
{
    return block_cast<Original::Stack> (data).Head ();
}

void Stack::Release (const void *_newHead) noexcept
{
    block_cast<Original::Stack> (data).Release (_newHead);
}

void Stack::Clear () noexcept
{
    block_cast<Original::Stack> (data).Clear ();
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

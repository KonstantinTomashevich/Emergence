#include <API/Common/BlockCast.hpp>

#include <Memory/Original/Stack.hpp>
#include <Memory/Stack.hpp>

namespace Emergence::Memory
{
Stack::Stack (Profiler::AllocationGroup _group, std::size_t _capacity) noexcept
{
    new (&data) Original::Stack (std::move (_group), _capacity);
}

Stack::Stack (Stack &&_other) noexcept
{
    new (&data) Original::Stack (std::move (block_cast<Original::Stack> (_other.data)));
}

Stack::~Stack () noexcept
{
    block_cast<Original::Stack> (data).~Stack ();
}

void *Stack::Acquire (std::size_t _chunkSize, std::uintptr_t _alignAs) noexcept
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

std::size_t Stack::GetFreeSize () const noexcept
{
    return block_cast<Original::Stack> (data).GetFreeSize ();
}

const Profiler::AllocationGroup &Stack::GetAllocationGroup () const noexcept
{
    return block_cast<Original::Stack> (data).GetAllocationGroup ();
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

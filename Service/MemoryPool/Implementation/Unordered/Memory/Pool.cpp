#include <Memory/Pool.hpp>
#include <Memory/UnorderedPool.hpp>

#include <SyntaxSugar/BlockCast.hpp>

namespace Emergence::Memory
{
static constexpr std::size_t DEFAULT_PAGE_SIZE = 4096u;

Pool::Pool (std::size_t _chunkSize) noexcept
    : Pool (_chunkSize, DEFAULT_PAGE_SIZE / _chunkSize)
{
}

Pool::Pool (std::size_t _chunkSize, std::size_t _preferredPageCapacity) noexcept
{
    new (&data) UnorderedPool (_chunkSize, _preferredPageCapacity);
}

Pool::Pool (Pool &&_other) noexcept
{
    new (&data) UnorderedPool (std::move (block_cast <UnorderedPool> (_other.data)));
}

Pool::~Pool () noexcept
{
    block_cast <UnorderedPool> (data).~UnorderedPool ();
}

void *Pool::Acquire () noexcept
{
    return block_cast <UnorderedPool> (data).Acquire ();
}

void Pool::Release (void *_chunk) noexcept
{
    block_cast <UnorderedPool> (data).Release (_chunk);
}

void Pool::Shrink () noexcept
{
    block_cast <UnorderedPool> (data).Shrink ();
}

void Pool::Clear () noexcept
{
    block_cast <UnorderedPool> (data).Clear ();
}

std::size_t Pool::GetAllocatedSpace () const noexcept
{
    return block_cast <UnorderedPool> (data).GetAllocatedSpace ();
}

Pool &Pool::operator = (Pool &&_other) noexcept
{
    if (this != &_other)
    {
        this->~Pool ();
        new (this) Pool (std::move (_other));
    }

    return *this;
}
} // namespace Emergence::Memory

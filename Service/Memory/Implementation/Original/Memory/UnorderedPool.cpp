#include <Memory/Original/UnorderedPool.hpp>
#include <Memory/UnorderedPool.hpp>

#include <SyntaxSugar/BlockCast.hpp>

namespace Emergence::Memory
{
static constexpr std::size_t DEFAULT_PAGE_SIZE = 4096u;

UnorderedPool::UnorderedPool (Profiler::AllocationGroup _group, std::size_t _chunkSize, std::size_t _alignment) noexcept
    : UnorderedPool (std::move (_group), _chunkSize, _alignment, DEFAULT_PAGE_SIZE / _chunkSize)
{
}

UnorderedPool::UnorderedPool (Profiler::AllocationGroup _group,
                              std::size_t _chunkSize,
                              std::size_t _alignment,
                              std::size_t _preferredPageCapacity) noexcept
{
    new (&data) Original::UnorderedPool (std::move (_group), _chunkSize, _alignment, _preferredPageCapacity);
}

UnorderedPool::UnorderedPool (UnorderedPool &&_other) noexcept
{
    new (&data) Original::UnorderedPool (std::move (block_cast<Original::UnorderedPool> (_other.data)));
}

UnorderedPool::~UnorderedPool () noexcept
{
    block_cast<Original::UnorderedPool> (data).~UnorderedPool ();
}

void *UnorderedPool::Acquire () noexcept
{
    return block_cast<Original::UnorderedPool> (data).Acquire ();
}

void UnorderedPool::Release (void *_chunk) noexcept
{
    block_cast<Original::UnorderedPool> (data).Release (_chunk);
}

void UnorderedPool::Clear () noexcept
{
    block_cast<Original::UnorderedPool> (data).Clear ();
}

bool UnorderedPool::IsEmpty () const noexcept
{
    return block_cast<Original::UnorderedPool> (data).IsEmpty ();
}

const Profiler::AllocationGroup &UnorderedPool::GetAllocationGroup () const noexcept
{
    return block_cast<Original::UnorderedPool> (data).GetAllocationGroup ();
}

UnorderedPool &UnorderedPool::operator= (UnorderedPool &&_other) noexcept
{
    if (this != &_other)
    {
        this->~UnorderedPool ();
        new (this) UnorderedPool (std::move (_other));
    }

    return *this;
}
} // namespace Emergence::Memory

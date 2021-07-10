#include <Memory/Pool.hpp>
#include <Memory/UnorderedPool.hpp>

#include <SyntaxSugar/BlockCast.hpp>

namespace Emergence::Memory
{
Pool::AcquiredChunkConstIterator::~AcquiredChunkConstIterator () noexcept
{
    block_cast <UnorderedPool::AcquiredChunkConstIterator> (data).~AcquiredChunkConstIterator ();
}

const void *Pool::AcquiredChunkConstIterator::operator * () const noexcept
{
    return *block_cast <UnorderedPool::AcquiredChunkConstIterator> (data);
}

Pool::AcquiredChunkConstIterator &Pool::AcquiredChunkConstIterator::operator ++ () noexcept
{
    ++block_cast <UnorderedPool::AcquiredChunkConstIterator> (data);
    return *this;
}

Pool::AcquiredChunkConstIterator Pool::AcquiredChunkConstIterator::operator ++ (int) noexcept
{
    UnorderedPool::AcquiredChunkConstIterator result =
        block_cast <UnorderedPool::AcquiredChunkConstIterator> (data)++;
    return AcquiredChunkConstIterator (reinterpret_cast <decltype (data) *> (&result));
}

bool Pool::AcquiredChunkConstIterator::operator == (const Pool::AcquiredChunkConstIterator &_other) const noexcept
{
    return block_cast <UnorderedPool::AcquiredChunkConstIterator> (data) ==
           block_cast <UnorderedPool::AcquiredChunkConstIterator> (_other.data);
}

bool Pool::AcquiredChunkConstIterator::operator != (const Pool::AcquiredChunkConstIterator &_other) const noexcept
{
    return !(*this == _other);
}

Pool::AcquiredChunkConstIterator::AcquiredChunkConstIterator (const std::array <uint8_t, DATA_MAX_SIZE> *_data) noexcept
{
    new (&data) UnorderedPool::AcquiredChunkConstIterator (
        block_cast <UnorderedPool::AcquiredChunkConstIterator> (*_data));
}

Pool::AcquiredChunkIterator::~AcquiredChunkIterator () noexcept
{
    block_cast <UnorderedPool::AcquiredChunkIterator> (data).~AcquiredChunkIterator ();
}

void *Pool::AcquiredChunkIterator::operator * () const noexcept
{
    return *block_cast <UnorderedPool::AcquiredChunkIterator> (data);
}

Pool::AcquiredChunkIterator &Pool::AcquiredChunkIterator::operator ++ () noexcept
{
    ++block_cast <UnorderedPool::AcquiredChunkIterator> (data);
    return *this;
}

Pool::AcquiredChunkIterator Pool::AcquiredChunkIterator::operator ++ (int) noexcept
{
    UnorderedPool::AcquiredChunkIterator result = block_cast <UnorderedPool::AcquiredChunkIterator> (data)++;
    return AcquiredChunkIterator (reinterpret_cast <decltype (data) *> (&result));
}

bool Pool::AcquiredChunkIterator::operator == (const Pool::AcquiredChunkIterator &_other) const noexcept
{
    return block_cast <UnorderedPool::AcquiredChunkIterator> (data) ==
           block_cast <UnorderedPool::AcquiredChunkIterator> (_other.data);
}

bool Pool::AcquiredChunkIterator::operator != (const Pool::AcquiredChunkIterator &_other) const noexcept
{
    return !(*this == _other);
}

Pool::AcquiredChunkIterator::AcquiredChunkIterator (const std::array <uint8_t, DATA_MAX_SIZE> *_data) noexcept
{
    new (&data) UnorderedPool::AcquiredChunkIterator (
        block_cast <UnorderedPool::AcquiredChunkIterator> (*_data));
}

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

Pool::AcquiredChunkConstIterator Pool::BeginAcquired () const noexcept
{
    UnorderedPool::AcquiredChunkConstIterator iterator = block_cast <UnorderedPool> (data).BeginAcquired ();
    return AcquiredChunkConstIterator (
        reinterpret_cast <const decltype (AcquiredChunkConstIterator::data) *> (&iterator));
}

Pool::AcquiredChunkConstIterator Pool::EndAcquired () const noexcept
{
    UnorderedPool::AcquiredChunkConstIterator iterator = block_cast <UnorderedPool> (data).EndAcquired ();
    return AcquiredChunkConstIterator (
        reinterpret_cast <const decltype (AcquiredChunkConstIterator::data) *> (&iterator));
}

Pool::AcquiredChunkIterator Pool::BeginAcquired () noexcept
{
    UnorderedPool::AcquiredChunkIterator iterator = block_cast <UnorderedPool> (data).BeginAcquired ();
    return AcquiredChunkIterator (reinterpret_cast <const decltype (AcquiredChunkIterator::data) *> (&iterator));
}

Pool::AcquiredChunkIterator Pool::EndAcquired () noexcept
{
    UnorderedPool::AcquiredChunkIterator iterator = block_cast <UnorderedPool> (data).EndAcquired ();
    return AcquiredChunkIterator (reinterpret_cast <const decltype (AcquiredChunkIterator::data) *> (&iterator));
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

Pool::AcquiredChunkConstIterator begin (const Pool &_pool) noexcept
{
    return _pool.BeginAcquired ();
}

Pool::AcquiredChunkConstIterator end (const Pool &_pool) noexcept
{
    return _pool.EndAcquired ();
}

Pool::AcquiredChunkIterator begin (Pool &_pool) noexcept
{
    return _pool.BeginAcquired ();
}

Pool::AcquiredChunkIterator end (Pool &_pool) noexcept
{
    return _pool.EndAcquired ();
}
} // namespace Emergence::Memory

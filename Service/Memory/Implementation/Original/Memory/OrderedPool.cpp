#include <API/Common/Implementation/Iterator.hpp>

#include <Memory/OrderedPool.hpp>
#include <Memory/Original/OrderedPool.hpp>

#include <SyntaxSugar/BlockCast.hpp>

namespace Emergence::Memory
{
using AcquiredChunkConstIterator = OrderedPool::AcquiredChunkConstIterator;

using AcquiredChunkConstIteratorImplementation = Original::OrderedPool::AcquiredChunkConstIterator;

EMERGENCE_BIND_FORWARD_ITERATOR_OPERATIONS_IMPLEMENTATION (AcquiredChunkConstIterator,
                                                           AcquiredChunkConstIteratorImplementation)

const void *OrderedPool::AcquiredChunkConstIterator::operator* () const noexcept
{
    return *block_cast<Original::OrderedPool::AcquiredChunkConstIterator> (data);
}

using AcquiredChunkIterator = OrderedPool::AcquiredChunkIterator;

using AcquiredChunkIteratorImplementation = Original::OrderedPool::AcquiredChunkIterator;

EMERGENCE_BIND_FORWARD_ITERATOR_OPERATIONS_IMPLEMENTATION (AcquiredChunkIterator, AcquiredChunkIteratorImplementation)

void *OrderedPool::AcquiredChunkIterator::operator* () const noexcept
{
    return *block_cast<Original::OrderedPool::AcquiredChunkIterator> (data);
}

static constexpr std::size_t DEFAULT_PAGE_SIZE = 4096u;

OrderedPool::OrderedPool (Profiler::AllocationGroup _group, std::size_t _chunkSize, std::size_t _alignment) noexcept
    : OrderedPool (std::move (_group), _chunkSize, _alignment, DEFAULT_PAGE_SIZE / _chunkSize)
{
}

OrderedPool::OrderedPool (Profiler::AllocationGroup _group,
                          std::size_t _chunkSize,
                          std::size_t _alignment,
                          std::size_t _preferredPageCapacity) noexcept
{
    new (&data) Original::OrderedPool (std::move (_group), _chunkSize, _alignment, _preferredPageCapacity);
}

OrderedPool::OrderedPool (OrderedPool &&_other) noexcept
{
    new (&data) Original::OrderedPool (std::move (block_cast<Original::OrderedPool> (_other.data)));
}

OrderedPool::~OrderedPool () noexcept
{
    block_cast<Original::OrderedPool> (data).~OrderedPool ();
}

void *OrderedPool::Acquire () noexcept
{
    return block_cast<Original::OrderedPool> (data).Acquire ();
}

void OrderedPool::Release (void *_chunk) noexcept
{
    block_cast<Original::OrderedPool> (data).Release (_chunk);
}

void OrderedPool::Shrink () noexcept
{
    block_cast<Original::OrderedPool> (data).Shrink ();
}

void OrderedPool::Clear () noexcept
{
    block_cast<Original::OrderedPool> (data).Clear ();
}

bool OrderedPool::IsEmpty () const noexcept
{
    return block_cast<Original::OrderedPool> (data).IsEmpty ();
}

OrderedPool::AcquiredChunkConstIterator OrderedPool::BeginAcquired () const noexcept
{
    auto iterator = block_cast<Original::OrderedPool> (data).BeginAcquired ();
    return AcquiredChunkConstIterator (
        reinterpret_cast<const decltype (AcquiredChunkConstIterator::data) *> (&iterator));
}

OrderedPool::AcquiredChunkConstIterator OrderedPool::EndAcquired () const noexcept
{
    auto iterator = block_cast<Original::OrderedPool> (data).EndAcquired ();
    return AcquiredChunkConstIterator (
        reinterpret_cast<const decltype (AcquiredChunkConstIterator::data) *> (&iterator));
}

OrderedPool::AcquiredChunkIterator OrderedPool::BeginAcquired () noexcept
{
    auto iterator = block_cast<Original::OrderedPool> (data).BeginAcquired ();
    return AcquiredChunkIterator (reinterpret_cast<const decltype (AcquiredChunkIterator::data) *> (&iterator));
}

OrderedPool::AcquiredChunkIterator OrderedPool::EndAcquired () noexcept
{
    auto iterator = block_cast<Original::OrderedPool> (data).EndAcquired ();
    return AcquiredChunkIterator (reinterpret_cast<const decltype (AcquiredChunkIterator::data) *> (&iterator));
}

const Profiler::AllocationGroup &OrderedPool::GetAllocationGroup () const noexcept
{
    return block_cast<Original::OrderedPool> (data).GetAllocationGroup ();
}

OrderedPool &OrderedPool::operator= (OrderedPool &&_other) noexcept
{
    if (this != &_other)
    {
        this->~OrderedPool ();
        new (this) OrderedPool (std::move (_other));
    }

    return *this;
}

OrderedPool::AcquiredChunkConstIterator begin (const OrderedPool &_pool) noexcept
{
    return _pool.BeginAcquired ();
}

OrderedPool::AcquiredChunkConstIterator end (const OrderedPool &_pool) noexcept
{
    return _pool.EndAcquired ();
}

OrderedPool::AcquiredChunkIterator begin (OrderedPool &_pool) noexcept
{
    return _pool.BeginAcquired ();
}

OrderedPool::AcquiredChunkIterator end (OrderedPool &_pool) noexcept
{
    return _pool.EndAcquired ();
}
} // namespace Emergence::Memory

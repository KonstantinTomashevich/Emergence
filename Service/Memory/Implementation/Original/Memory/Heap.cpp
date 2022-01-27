#include <Memory/Heap.hpp>

#include <SyntaxSugar/BlockCast.hpp>

namespace Emergence::Memory
{
Heap::Heap (Profiler::AllocationGroup _group) noexcept
{
    new (&data) Profiler::AllocationGroup (std::move (_group));
}

Heap::Heap (Heap &&_other) noexcept
{
    new (&data) Profiler::AllocationGroup (std::move (block_cast<Profiler::AllocationGroup> (_other.data)));
}

Heap::~Heap () noexcept
{
    block_cast<Profiler::AllocationGroup> (data).~AllocationGroup ();
}

void *Heap::Acquire (size_t _bytes) noexcept
{
    auto &registry = block_cast<Profiler::AllocationGroup> (data);
    registry.Allocate (_bytes);
    registry.Acquire (_bytes);
    return malloc (_bytes);
}

void *Heap::Resize (void *_record, size_t _currentSize, size_t _newSize) noexcept
{
    auto &registry = block_cast<Profiler::AllocationGroup> (data);
    registry.Allocate (_newSize);
    registry.Acquire (_newSize);
    registry.Release (_currentSize);
    registry.Free (_currentSize);
    return realloc (_record, _newSize);
}

void Heap::Release (void *_record, size_t _bytes) noexcept
{
    auto &registry = block_cast<Profiler::AllocationGroup> (data);
    registry.Release (_bytes);
    registry.Free (_bytes);
    free (_record);
}

const Profiler::AllocationGroup &Heap::GetAllocationGroup () const noexcept
{
    return block_cast<Profiler::AllocationGroup> (data);
}

Heap &Heap::operator= (Heap &&_other) noexcept
{
    block_cast<Profiler::AllocationGroup> (data) = std::move (block_cast<Profiler::AllocationGroup> (_other.data));
    return *this;
}
} // namespace Emergence::Memory

#include <Memory/Heap.hpp>
#include <Memory/Profiler/Registry.hpp>

#include <SyntaxSugar/BlockCast.hpp>

namespace Emergence::Memory
{
Heap::Heap (UniqueString _groupId) noexcept
{
    new (&data) Profiler::Registry (_groupId);
}

Heap::Heap (Heap &&_other) noexcept
{
    new (&data) Profiler::Registry (std::move (block_cast<Profiler::Registry> (_other.data)));
}

Heap::~Heap () noexcept
{
    block_cast<Profiler::Registry> (data).~Registry ();
}

void *Heap::Acquire (size_t _bytes) noexcept
{
    auto &registry = block_cast<Profiler::Registry> (data);
    registry.Allocate (_bytes);
    registry.Acquire (_bytes);
    return malloc (_bytes);
}

void *Heap::Resize (void *_record, size_t _currentSize, size_t _newSize) noexcept
{
    auto &registry = block_cast<Profiler::Registry> (data);
    registry.Allocate (_newSize);
    registry.Acquire (_newSize);
    registry.Release (_currentSize);
    registry.Free (_currentSize);
    return realloc (_record, _newSize);
}

void Heap::Release (void *_record, size_t _bytes) noexcept
{
    auto &registry = block_cast<Profiler::Registry> (data);
    registry.Release (_bytes);
    registry.Free (_bytes);
    free (_record);
}

Heap &Heap::operator= (Heap &&_other) noexcept
{
    block_cast<Profiler::Registry> (data) = std::move (block_cast<Profiler::Registry> (_other.data));
    return *this;
}

UniqueString Heap::GetGroupId () const noexcept
{
    return block_cast<Profiler::Registry> (data).GetId ();
}
} // namespace Emergence::Memory

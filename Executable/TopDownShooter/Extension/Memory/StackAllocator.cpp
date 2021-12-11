#include <cassert>
#include <cstdlib>

#include <Memory/StackAllocator.hpp>

namespace Emergence::Memory
{
Memory::StackAllocator::StackAllocator (size_t _reservedMemory) noexcept : memoryBlockStart (malloc (_reservedMemory))
{
    memoryBlockEnd = static_cast<uint8_t *> (memoryBlockStart) + _reservedMemory;
    stackTop = memoryBlockStart;
}

Memory::StackAllocator::StackAllocator (StackAllocator &&_other) noexcept
    : memoryBlockStart (_other.memoryBlockStart),
      memoryBlockEnd (_other.memoryBlockEnd),
      stackTop (_other.stackTop)
{
    _other.memoryBlockStart = nullptr;
    _other.memoryBlockEnd = nullptr;
    _other.stackTop = nullptr;
}

Memory::StackAllocator::~StackAllocator ()
{
    free (memoryBlockStart);
}

StackAllocator::RollbackMarker Memory::StackAllocator::CreateRollbackMarker () const noexcept
{
    RollbackMarker marker;
    marker.point = stackTop;
    return marker;
}

void Memory::StackAllocator::Rollback (const StackAllocator::RollbackMarker &_marker) noexcept
{
    assert (_marker.point >= memoryBlockStart);
    assert (_marker.point < memoryBlockEnd);
    stackTop = _marker.point;
}

void *Memory::StackAllocator::Acquire (size_t _bytes) noexcept
{
    void *pointer = stackTop;
    stackTop = static_cast<uint8_t *> (stackTop) + _bytes;
    assert (stackTop < memoryBlockEnd);
    return pointer;
}
} // namespace Emergence::Memory

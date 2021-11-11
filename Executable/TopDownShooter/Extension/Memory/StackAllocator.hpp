#pragma once

#include <cstdint>

#include <API/Common/Shortcuts.hpp>

namespace Emergence::Memory
{
class StackAllocator final
{
public:
    class RollbackMarker final
    {
        friend class StackAllocator;

        void *point = nullptr;
    };

    explicit StackAllocator (size_t _reservedMemory) noexcept;

    StackAllocator (const StackAllocator &_other) = delete;

    StackAllocator (StackAllocator &&_other) noexcept;

    ~StackAllocator ();

    RollbackMarker CreateRollbackMarker () const noexcept;

    void Rollback (const RollbackMarker &_marker) noexcept;

    void *Acquire (size_t _bytes) noexcept;

    EMERGENCE_DELETE_ASSIGNMENT (StackAllocator);

private:
    void *memoryBlockStart = nullptr;
    void *memoryBlockEnd = nullptr;
    void *stackTop = nullptr;
};
} // namespace Emergence::Memory

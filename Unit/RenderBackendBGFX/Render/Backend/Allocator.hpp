#pragma once

#include <bx/allocator.h>

#include <Memory/Heap.hpp>

namespace Emergence::Render::Backend
{
Memory::Profiler::AllocationGroup GetAllocationGroup () noexcept;

class Allocator final : public bx::AllocatorI
{
public:
    void *realloc (void *_pointer,
                   std::size_t _size,
                   std::size_t _alignment,
                   const char * /*unused*/,
                   std::uint32_t /*unused*/) override;

private:
    Memory::Heap allocator {GetAllocationGroup ()};
};

bx::AllocatorI *GetProfiledAllocator () noexcept;

bx::AllocatorI *GetEfficientAllocator () noexcept;

void SetCurrentAllocator (bx::AllocatorI *_allocator) noexcept;

bx::AllocatorI *GetCurrentAllocator () noexcept;
} // namespace Emergence::Render::Backend

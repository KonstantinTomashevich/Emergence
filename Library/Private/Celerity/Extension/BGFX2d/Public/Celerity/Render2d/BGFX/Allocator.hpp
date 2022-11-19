#pragma once

#include <bx/allocator.h>

#include <Memory/Heap.hpp>

namespace Emergence::Celerity::BGFX
{
class Allocator final : public bx::AllocatorI
{
public:
    void *realloc (
        void *_pointer, size_t _size, size_t _alignment, const char * /*unused*/, uint32_t /*unused*/) override;

private:
    Memory::Heap allocator {Memory::Profiler::AllocationGroup {Memory::Profiler::AllocationGroup::Root (),
                                                               Memory::UniqueString {"Render2d::BGFX"}}};
};

bx::AllocatorI *GetProfiledAllocator () noexcept;

bx::AllocatorI *GetEfficientAllocator () noexcept;

void SetCurrentAllocator (bx::AllocatorI *_allocator) noexcept;

bx::AllocatorI *GetCurrentAllocator () noexcept;
} // namespace Emergence::Celerity::BGFX

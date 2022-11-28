#include <Render/Backend/Allocator.hpp>

namespace Emergence::Render::Backend
{
Memory::Profiler::AllocationGroup GetAllocationGroup () noexcept
{
    static Memory::Profiler::AllocationGroup group {Memory::Profiler::AllocationGroup::Root (),
                                                    Memory::UniqueString {"Render::Backend::BGFX"}};
    return group;
}

void *Allocator::realloc (void *_pointer, size_t _size, size_t _alignment, const char * /*unused*/, uint32_t /*unused*/)
{
    _alignment = std::max (_alignment, sizeof (uintptr_t));
    if (_size == 0u)
    {
        if (_pointer)
        {
            void *initialAddress = static_cast<uint8_t *> (_pointer) - _alignment;
            allocator.Release (initialAddress, *static_cast<uintptr_t *> (initialAddress));
        }

        return nullptr;
    }

    if (!_pointer)
    {
        void *allocated = allocator.Acquire (_size + _alignment, _alignment);
        *static_cast<uintptr_t *> (allocated) = _size;
        return static_cast<uint8_t *> (allocated) + _alignment;
    }

    void *initialAddress = static_cast<uint8_t *> (_pointer) - _alignment;
    void *newAddress =
        allocator.Resize (initialAddress, _alignment, *static_cast<uintptr_t *> (initialAddress), _size + _alignment);
    *static_cast<uintptr_t *> (newAddress) = _size;
    return static_cast<uint8_t *> (newAddress) + _alignment;
}

static bx::AllocatorI *currentAllocator = nullptr;

bx::AllocatorI *GetProfiledAllocator () noexcept
{
    static Allocator allocator;
    return &allocator;
}

bx::AllocatorI *GetEfficientAllocator () noexcept
{
    static bx::DefaultAllocator allocator;
    return &allocator;
}

void SetCurrentAllocator (bx::AllocatorI *_allocator) noexcept
{
    currentAllocator = _allocator;
}

bx::AllocatorI *GetCurrentAllocator () noexcept
{
    return currentAllocator;
}
} // namespace Emergence::Render::Backend

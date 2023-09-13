#include <imgui.h>

#include <Memory/Heap.hpp>

namespace Emergence::ImGUIAdapter
{
using namespace Memory::Literals;

static Memory::Heap &GetImGUIHeap ()
{
    static Memory::Heap heap {
        Memory::Profiler::AllocationGroup {Memory::Profiler::AllocationGroup::Top (), "ImGUI"_us}};
    return heap;
}

static void *ImGUIProfiledAllocate (std::size_t _amount, void * /*unused*/)
{
    auto *memory = static_cast<std::uintptr_t *> (
        GetImGUIHeap ().Acquire (_amount + sizeof (std::uintptr_t), alignof (std::uintptr_t)));
    *memory = static_cast<std::uintptr_t> (_amount);
    return memory + 1u;
}

static void ImGUIProfiledFree (void *_pointer, void * /*unused*/)
{
    if (_pointer)
    {
        auto *memory = static_cast<std::uintptr_t *> (_pointer) - 1u;
        GetImGUIHeap ().Release (memory, *memory);
    }
}

[[maybe_unused]] static const struct Initializer
{
    Initializer ()
    {
        ImGui::SetAllocatorFunctions (ImGUIProfiledAllocate, ImGUIProfiledFree, nullptr);
    }
} INITIALIZER;
} // namespace Emergence::ImGUIAdapter

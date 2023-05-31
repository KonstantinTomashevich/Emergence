#pragma once

#include <Celerity/Asset/Asset.hpp>

#include <Handling/Handle.hpp>
#include <Handling/HandleableBase.hpp>

namespace Emergence::Celerity
{
/// \brief Syntax sugar for asset loading states that are shared
///        with job dispatcher and therefore escape Celerity context.
/// \details Provides custom new and delete in order to implement memory profiling.
template <typename Successor>
class LoadingSharedState : public Handling::HandleableBase
{
public:
    void *operator new (std::size_t /*unused*/) noexcept;

    void operator delete (void *_pointer) noexcept;

    /// \brief Asset loading state is used as return value for processed asset loading jobs.
    std::atomic<AssetState> state {AssetState::LOADING};

protected:
    /// \brief Allocation group used by ::GetHeap.
    static Memory::Profiler::AllocationGroup GetAllocationGroup () noexcept;

    /// \brief Heap for allocating instances of this state.
    static Memory::Heap &GetHeap () noexcept;
};

template <typename Successor>
void *LoadingSharedState<Successor>::operator new (std::size_t /*unused*/) noexcept
{
    return GetHeap ().Acquire (sizeof (Successor), alignof (Successor));
}

template <typename Successor>
void LoadingSharedState<Successor>::operator delete (void *_pointer) noexcept
{
    GetHeap ().Release (_pointer, sizeof (Successor));
}

template <typename Successor>
Memory::Profiler::AllocationGroup LoadingSharedState<Successor>::GetAllocationGroup () noexcept
{
    using namespace Memory::Literals;
    return Memory::Profiler::AllocationGroup {
        {Memory::Profiler::AllocationGroup {Memory::Profiler::AllocationGroup::Top (), "Celerity"_us}, "Shared"_us},
        Memory::UniqueString {Successor::ALLOCATION_GROUP_NAME}};
}

template <typename Successor>
Memory::Heap &LoadingSharedState<Successor>::GetHeap () noexcept
{
    static Memory::Heap heap {GetAllocationGroup ()};
    return heap;
}
} // namespace Emergence::Celerity

#pragma once

#include <Assert/Assert.hpp>

#include <Celerity/WorldSingleton.hpp>

#include <Handling/Handle.hpp>
#include <Handling/HandleableBase.hpp>

namespace Emergence::Celerity
{
/// \brief Base class for objects that escape Celerity storage context.
/// \details Sometimes we need to let some data and some logic escape context of Celerity storage and pipelines.
///          For example, we might need to schedule background task like resource loading through JobDispatcher.
///          To do that safely, we need a handleable object that is shared between Celerity context and JobDispatcher.
///          This object is called context escape as it technically escapes Celerity execution context.
template <typename Successor>
class ContextEscape : public Handling::HandleableBase
{
public:
    void *operator new (std::size_t /*unused*/) noexcept;

    void operator delete (void *_pointer) noexcept;

    /// \brief Informs world that object got out of context and is now shared.
    void ReportEscaped (const WorldSingleton *_world) noexcept;

    /// \brief Informs that object is no longer shared and belongs only to Celerity context.
    void ReportReturned (const WorldSingleton *_world) noexcept;

protected:
    /// \brief Allocation group used by ::GetHeap.
    static Memory::Profiler::AllocationGroup GetAllocationGroup () noexcept;

    /// \brief Heap for allocating instances of this state.
    static Memory::Heap &GetHeap () noexcept;
};

template <typename Successor>
void *ContextEscape<Successor>::operator new (std::size_t /*unused*/) noexcept
{
    return GetHeap ().Acquire (sizeof (Successor), alignof (Successor));
}

template <typename Successor>
void ContextEscape<Successor>::operator delete (void *_pointer) noexcept
{
    GetHeap ().Release (_pointer, sizeof (Successor));
}

template <typename Successor>
void ContextEscape<Successor>::ReportEscaped (const WorldSingleton *_world) noexcept
{
    EMERGENCE_ASSERT (_world->contextEscapeAllowed);
    // We accept const world only to indicate that method is thread safe.
    ++const_cast<WorldSingleton *> (_world)->contextEscapeCounter;
}

template <typename Successor>
void ContextEscape<Successor>::ReportReturned (const WorldSingleton *_world) noexcept
{
    EMERGENCE_ASSERT (_world->contextEscapeCounter > 0u);
    // We accept const world only to indicate that method is thread safe.
    --const_cast<WorldSingleton *> (_world)->contextEscapeCounter;
}

template <typename Successor>
Memory::Profiler::AllocationGroup ContextEscape<Successor>::GetAllocationGroup () noexcept
{
    using namespace Memory::Literals;
    return Memory::Profiler::AllocationGroup {
        {Memory::Profiler::AllocationGroup {Memory::Profiler::AllocationGroup::Top (), "Celerity"_us}, "Shared"_us},
        Memory::UniqueString {Successor::ALLOCATION_GROUP_NAME}};
}

template <typename Successor>
Memory::Heap &ContextEscape<Successor>::GetHeap () noexcept
{
    static Memory::Heap heap {GetAllocationGroup ()};
    return heap;
}
} // namespace Emergence::Celerity

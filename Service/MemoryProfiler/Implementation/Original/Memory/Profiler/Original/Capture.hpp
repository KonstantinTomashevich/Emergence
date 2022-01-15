#pragma once

#include <atomic>

#include <Handling/Handle.hpp>
#include <Handling/HandleableBase.hpp>

#include <Memory/Profiler/Original/AllocationGroup.hpp>
#include <Memory/Profiler/Original/EventManager.hpp>
#include <Memory/Profiler/Original/ProfilingLock.hpp>

namespace Emergence::Memory::Profiler::Original
{
class CapturedAllocationGroup final : public Handling::HandleableBase
{
public:
    class Iterator final
    {
    public:
        EMERGENCE_FORWARD_ITERATOR_OPERATIONS (Iterator, Handling::Handle<CapturedAllocationGroup>);

    private:
        /// CapturedAllocationGroup constructs iterators.
        friend class CapturedAllocationGroup;

        explicit Iterator (Handling::Handle<CapturedAllocationGroup> _current) noexcept;

        Handling::Handle<CapturedAllocationGroup> current;
    };

    void *operator new (std::size_t /*unused*/) noexcept;

    void operator delete (void *_pointer) noexcept;

    CapturedAllocationGroup (const AllocationGroup &_source, const ProfilingLock &_lock) noexcept;

    CapturedAllocationGroup (const CapturedAllocationGroup &_other) = delete;

    CapturedAllocationGroup (CapturedAllocationGroup &&_other) = delete;

    ~CapturedAllocationGroup () = default;

    [[nodiscard]] Iterator BeginChildren () const noexcept;

    static Iterator EndChildren () noexcept;

    [[nodiscard]] UniqueString GetId () const noexcept;

    [[nodiscard]] size_t GetAcquired () const noexcept;

    [[nodiscard]] size_t GetReserved () const noexcept;

    [[nodiscard]] size_t GetTotal () const noexcept;

    [[nodiscard]] const AllocationGroup *GetSource () const noexcept;

    EMERGENCE_DELETE_ASSIGNMENT (CapturedAllocationGroup);

private:
    Memory::UniqueString id;
    std::size_t reserved = 0u;
    std::size_t acquired = 0u;

    Handling::Handle<CapturedAllocationGroup> firstChild;
    Handling::Handle<CapturedAllocationGroup> nextOnLevel;
    const AllocationGroup *source = nullptr;
};

class EventObserver final
{
public:
    EventObserver (const ProfilingLock &_lock) noexcept;

    EventObserver (const EventObserver &_other) = delete;

    EventObserver (EventObserver &&_other) noexcept;

    ~EventObserver () noexcept;

    const Event *NextEvent (const ProfilingLock &_lock) noexcept;

    EMERGENCE_DELETE_ASSIGNMENT (EventObserver);

private:
    const EventNode *current = nullptr;

    // TODO: Find more beautiful solution for moved out captures destruction?
    bool movedOut = false;
};
} // namespace Emergence::Memory::Profiler::Original

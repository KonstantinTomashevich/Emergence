#pragma once

#include <chrono>
#include <cstdint>

#include <API/Common/ImplementationBinding.hpp>
#include <API/Common/Iterator.hpp>
#include <API/Common/Shortcuts.hpp>

#include <Memory/Profiler/AllocationGroup.hpp>
#include <Memory/UniqueString.hpp>

namespace Emergence::Memory::Profiler
{
enum class EventType
{
    ALLOCATE = 0,
    ACQUIRE,
    RELEASE,
    FREE,
    MARKER
};

struct Event final
{
    EventType type = EventType::ALLOCATE;

    AllocationGroup group;

    std::chrono::time_point<std::chrono::high_resolution_clock> time;

    union
    {
        size_t bytes = 0u;
        UniqueString markerId;
    };
};

void AddMarker (UniqueString _markerId, AllocationGroup _group = AllocationGroup::Root ()) noexcept;

class CapturedAllocationGroup final
{
public:
    class Iterator final
    {
    public:
        EMERGENCE_FORWARD_ITERATOR_OPERATIONS (Iterator, CapturedAllocationGroup);

    private:
        /// CapturedAllocationGroup constructs iterators.
        friend class CapturedAllocationGroup;

        EMERGENCE_BIND_IMPLEMENTATION_INPLACE (sizeof (uintptr_t));

        explicit Iterator (const std::array<uint8_t, DATA_MAX_SIZE> *_data) noexcept;
    };

    CapturedAllocationGroup (const CapturedAllocationGroup &_other) noexcept;

    CapturedAllocationGroup (CapturedAllocationGroup &&_other) noexcept;

    ~CapturedAllocationGroup () noexcept;

    [[nodiscard]] Iterator BeginChildren () const noexcept;

    [[nodiscard]] Iterator EndChildren () const noexcept;

    [[nodiscard]] UniqueString GetId () const noexcept;

    [[nodiscard]] size_t GetAcquired () const noexcept;

    [[nodiscard]] size_t GetReserved () const noexcept;

    [[nodiscard]] size_t GetTotal () const noexcept;

    EMERGENCE_DELETE_ASSIGNMENT (CapturedAllocationGroup);

private:
    friend class Capture;

    EMERGENCE_BIND_IMPLEMENTATION_HANDLE ();

    explicit CapturedAllocationGroup (void *_handle) noexcept;
};

class EventObserver
{
public:
    /// Copying captures seems counter-intuitive.
    EventObserver (const EventObserver &_other) = delete;

    EventObserver (EventObserver &&_other) noexcept;

    ~EventObserver () = default;

    const Event *NextEvent () noexcept;

    /// Assigning captures seems counter-intuitive.
    EMERGENCE_DELETE_ASSIGNMENT (EventObserver);

private:
    friend class Capture;

    EMERGENCE_BIND_IMPLEMENTATION_INPLACE (sizeof (uintptr_t) * 2u);

    explicit EventObserver (std::array<uint8_t, DATA_MAX_SIZE> *_data) noexcept;
};

class Capture final
{
public:
    Capture () = delete;

    static std::pair<CapturedAllocationGroup, EventObserver> Start () noexcept;
};
} // namespace Emergence::Memory::Profiler

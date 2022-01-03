#pragma once

#include <chrono>
#include <cstdint>

#include <API/Common/ImplementationBinding.hpp>
#include <API/Common/Iterator.hpp>
#include <API/Common/Shortcuts.hpp>

#include <Memory/UniqueString.hpp>

namespace Emergence::Memory::Profiler
{
enum class EventType
{
    ALLOCATE = 0,
    ACQUIRE,
    RELEASE,
    FREE
};

struct Event final
{
    // TODO: We can not just use group IDs, because they are hierarchical.
    uint64_t groupUID = 0u;

    EventType type = EventType::ALLOCATE;

    size_t bytes = 0u;

    std::chrono::time_point<std::chrono::high_resolution_clock> time;

    Event *next = nullptr;

    size_t startOfCapture = 0u;
};

class CapturedGroup final
{
public:
    class Iterator final
    {
    public:
        EMERGENCE_FORWARD_ITERATOR_OPERATIONS (Iterator, CapturedGroup);

    private:
        /// CapturedGroup constructs iterators.
        friend class CapturedGroup;

        EMERGENCE_BIND_IMPLEMENTATION_INPLACE (sizeof (uintptr_t));

        explicit Iterator (const std::array<uint8_t, DATA_MAX_SIZE> *_data) noexcept;
    };

    Iterator BeginChildren () const noexcept;

    Iterator EndChildren () const noexcept;

    UniqueString GetId () const noexcept;

    size_t GetAcquired () const noexcept;

    size_t GetReserved () const noexcept;

    size_t GetTotal () const noexcept;

private:
    friend class Capture;

    CapturedGroup (const void *_handle) noexcept;

    EMERGENCE_BIND_IMPLEMENTATION_HANDLE ();
};

class Capture
{
public:
    Capture () noexcept;

    /// Copying captures seems counter-intuitive.
    Capture (const Capture &_other) = delete;

    Capture (Capture &&_other) noexcept;

    ~Capture () = default;

    const Event &EventBegin () const noexcept;

    const Event &EventCurrent () const noexcept;

    const Event *GoToNextEvent () noexcept;

    const Event *GoToPreviousEvent () noexcept;

    CapturedGroup InitialRoot () const noexcept;

    CapturedGroup GetInitialGroupByUID (uint64_t _uid) const noexcept;

    CapturedGroup CurrentRoot () const noexcept;

    CapturedGroup GetCurrentGroupByUID (uint64_t _uid) const noexcept;

    /// Assigning captures seems counter-intuitive.
    EMERGENCE_DELETE_ASSIGNMENT (Capture);

private:
    EMERGENCE_BIND_IMPLEMENTATION_HANDLE ();
};
} // namespace Emergence::Memory::Profiler

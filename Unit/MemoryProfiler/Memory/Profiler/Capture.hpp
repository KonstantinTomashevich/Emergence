#pragma once

#include <MemoryProfilerApi.hpp>

#include <cstdint>

#include <API/Common/ImplementationBinding.hpp>
#include <API/Common/Iterator.hpp>
#include <API/Common/Shortcuts.hpp>

#include <Memory/Profiler/AllocationGroup.hpp>
#include <Memory/UniqueString.hpp>

namespace Emergence::Memory::Profiler
{
/// \brief Enumerates all types of Event.
enum class EventType
{
    /// \brief Memory is allocated for future usage.
    ALLOCATE = 0,

    /// \brief Memory is being used for something.
    ACQUIRE,

    /// \brief Memory is no longer used.
    RELEASE,

    /// \brief Memory is no longer owned.
    FREE,

    /// \brief Message, used to make capture events array more readable.
    MARKER
};

/// \brief Every memory management operation produces appropriate event.
struct MemoryProfilerApi Event final
{
    EventType type = EventType::ALLOCATE;

    /// \brief Allocation group, associated with this event.
    AllocationGroup group;

    /// \brief Time point in nanoseconds after startup, at which event occurred.
    std::uint64_t timeNs;

    union
    {
        /// \brief Amount of bytes, used by operation, associated with ::type.
        /// \invariant ::type is EventType::ALLOCATE, EventType::ACQUIRE, EventType::RELEASE or EventType::Free.
        std::size_t bytes = 0u;

        /// \brief Marker id.
        /// \invariant ::type is EventType::MARKER.
        UniqueString markerId;
    };
};

/// \brief Creates new EventType::MARKER event, associated with given group.
/// \details Thread safe.
MemoryProfilerApi void AddMarker (UniqueString _markerId,
                                  const AllocationGroup &_group = AllocationGroup::Root ()) noexcept;

/// \brief State of allocation group, that was captured during Capture::Start.
class MemoryProfilerApi CapturedAllocationGroup final
{
public:
    /// \brief Provides iteration over captured states of allocation group children.
    class Iterator final
    {
    public:
        EMERGENCE_FORWARD_ITERATOR_OPERATIONS (Iterator, CapturedAllocationGroup);

    private:
        /// CapturedAllocationGroup constructs iterators.
        friend class CapturedAllocationGroup;

        EMERGENCE_BIND_IMPLEMENTATION_INPLACE (sizeof (std::uintptr_t));

        explicit Iterator (const std::array<std::uint8_t, DATA_MAX_SIZE> &_data) noexcept;
    };

    CapturedAllocationGroup (const CapturedAllocationGroup &_other) noexcept;

    CapturedAllocationGroup (CapturedAllocationGroup &&_other) noexcept;

    // NOLINTNEXTLINE(performance-trivially-destructible): Trivially destructible only for None implementation.
    ~CapturedAllocationGroup () noexcept;

    [[nodiscard]] Iterator BeginChildren () const noexcept;

    [[nodiscard]] Iterator EndChildren () const noexcept;

    /// \return Id of this group, unique among its siblings.
    [[nodiscard]] UniqueString GetId () const noexcept;

    /// \return Amount of bytes, that are used by any object.
    [[nodiscard]] std::size_t GetAcquired () const noexcept;

    /// \return Amount of bytes, that are not used by any object, but reserved for usage in future.
    [[nodiscard]] std::size_t GetReserved () const noexcept;

    /// \return Total amount of bytes, that are owned by allocation group.
    [[nodiscard]] std::size_t GetTotal () const noexcept;

    /// \return Allocation group, from which this capture was created.
    [[nodiscard]] AllocationGroup GetSource () const noexcept;

    /// \return Time from startup to capture of this group hierarchy in nanoseconds.
    [[nodiscard]] std::uint64_t GetCaptureTimeNs () const noexcept;

    EMERGENCE_DELETE_ASSIGNMENT (CapturedAllocationGroup);

private:
    friend class Capture;

    EMERGENCE_BIND_IMPLEMENTATION_HANDLE ()

    explicit CapturedAllocationGroup (void *_handle) noexcept;
};

/// \brief Allows user to read memory management events that occurred after Capture::Start.
class MemoryProfilerApi EventObserver
{
public:
    /// Copying captures seems counter-intuitive.
    EventObserver (const EventObserver &_other) = delete;

    EventObserver (EventObserver &&_other) noexcept;

    // NOLINTNEXTLINE(performance-trivially-destructible): Trivially destructible only for None implementation.
    ~EventObserver () noexcept;

    /// \return Next event or `nullptr` if there is no new events.
    /// \details Thread safe.
    const Event *NextEvent () noexcept;

    /// Assigning captures seems counter-intuitive.
    EMERGENCE_DELETE_ASSIGNMENT (EventObserver);

private:
    friend class Capture;

    EMERGENCE_BIND_IMPLEMENTATION_INPLACE (sizeof (std::uintptr_t) * 2u);

    explicit EventObserver (std::array<std::uint8_t, DATA_MAX_SIZE> &_data) noexcept;
};

/// \brief Provides API for capturing memory consumption.
class MemoryProfilerApi Capture final
{
public:
    Capture () = delete;

    /// \brief Capture state of all existing allocation groups and create
    ///        event observer for memory consumption changes tracking.
    /// \details Thread safe.
    static std::pair<CapturedAllocationGroup, EventObserver> Start () noexcept;
};
} // namespace Emergence::Memory::Profiler

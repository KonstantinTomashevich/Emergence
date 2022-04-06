#pragma once

#include <atomic>

#include <Memory/Profiler/Capture.hpp>
#include <Memory/Profiler/Original/ProfilingLock.hpp>
#include <Memory/UnorderedPool.hpp>

namespace Emergence::Memory::Profiler::Original
{
struct EventNode final
{
public:
    Event event;

private:
    friend class EventManager;

    explicit EventNode (Event _event) noexcept;

    EventNode *next = nullptr;
    size_t observers = 0u;
};

class EventManager final
{
public:
    static EventManager &Get () noexcept;

    EventManager (const EventManager &_other) = delete;

    EventManager (EventManager &&_other) = delete;

    void Allocate (const Profiler::AllocationGroup &_group, size_t _bytes, const ProfilingLock & /*unused*/) noexcept;

    void Acquire (const Profiler::AllocationGroup &_group, size_t _bytes, const ProfilingLock & /*unused*/) noexcept;

    void Release (const Profiler::AllocationGroup &_group, size_t _bytes, const ProfilingLock & /*unused*/) noexcept;

    void Free (const Profiler::AllocationGroup &_group, size_t _bytes, const ProfilingLock & /*unused*/) noexcept;

    void Marker (const Profiler::AllocationGroup &_group,
                 UniqueString _markerId,
                 const ProfilingLock & /*unused*/) noexcept;

    /// \return Node, that should be used as parameter for ::RequestNext during first next event request by user.
    const EventNode *StartObservation (const ProfilingLock & /*unused*/) noexcept;

    /// \return If movement to the next node was done, returns pointer to that node. Otherwise returns `nullptr`.
    const EventNode *RequestNext (const EventNode *_current, const ProfilingLock & /*unused*/) noexcept;

    void FinishObservation (const EventNode *_current, const ProfilingLock & /*unused*/) noexcept;

    EMERGENCE_DELETE_ASSIGNMENT (EventManager);

private:
    EventManager () = default;

    ~EventManager () = default;

    void RegisterNode (EventNode *_node) noexcept;

    void DropOutdatedEvents () noexcept;

    // We use stub allocation group, otherwise program will be stuck in recursive
    // event allocation: we will be creating events about allocation of events.
    UnorderedPool events {Profiler::AllocationGroup {}, sizeof (EventNode), alignof (EventNode)};

    EventNode *first = nullptr;
    EventNode *last = nullptr;

    /// \brief Number of active observers. If there is no observers, event creation will be skipped.
    size_t observers = 0u;

    /// \brief Number of freshly constructed observers, that have requested their first event yet.
    size_t freshObservers = 0u;
};
} // namespace Emergence::Memory::Profiler::Original

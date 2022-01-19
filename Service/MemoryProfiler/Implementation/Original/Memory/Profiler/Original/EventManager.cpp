#include <cassert>

#include <Memory/Profiler/Original/EventManager.hpp>

#include <SyntaxSugar/Time.hpp>

namespace Emergence::Memory::Profiler::Original
{
EventNode::EventNode (Event _event) noexcept : event (std::move (_event))
{
}

EventManager &EventManager::Get () noexcept
{
    static EventManager manager;
    return manager;
}

void EventManager::Allocate (const Profiler::AllocationGroup &_group,
                             size_t _bytes,
                             const ProfilingLock & /*unused*/) noexcept
{
    if (observers > 0u)
    {
        RegisterNode (new (events.Acquire ()) EventNode {Event {
            .type = EventType::ALLOCATE,
            .group = _group,
            .timeNs = Time::NanosecondsSinceStartup (),
            .bytes = _bytes,
        }});
    }
}

void EventManager::Acquire (const Profiler::AllocationGroup &_group,
                            size_t _bytes,
                            const ProfilingLock & /*unused*/) noexcept
{
    if (observers > 0u)
    {
        RegisterNode (new (events.Acquire ()) EventNode {Event {
            .type = EventType::ACQUIRE,
            .group = _group,
            .timeNs = Time::NanosecondsSinceStartup (),
            .bytes = _bytes,
        }});
    }
}

void EventManager::Release (const Profiler::AllocationGroup &_group,
                            size_t _bytes,
                            const ProfilingLock & /*unused*/) noexcept
{
    if (observers > 0u)
    {
        RegisterNode (new (events.Acquire ()) EventNode {Event {
            .type = EventType::RELEASE,
            .group = _group,
            .timeNs = Time::NanosecondsSinceStartup (),
            .bytes = _bytes,
        }});
    }
}

void EventManager::Free (const Profiler::AllocationGroup &_group,
                         size_t _bytes,
                         const ProfilingLock & /*unused*/) noexcept
{
    if (observers > 0u)
    {
        RegisterNode (new (events.Acquire ()) EventNode {Event {
            .type = EventType::FREE,
            .group = _group,
            .timeNs = Time::NanosecondsSinceStartup (),
            .bytes = _bytes,
        }});
    }
}

void EventManager::Marker (const Profiler::AllocationGroup &_group,
                           UniqueString _markerId,
                           const ProfilingLock & /*unused*/) noexcept
{
    if (observers > 0u)
    {
        RegisterNode (new (events.Acquire ()) EventNode {Event {
            .type = EventType::MARKER,
            .group = _group,
            .timeNs = Time::NanosecondsSinceStartup (),
            .markerId = _markerId,
        }});
    }
}

const EventNode *EventManager::StartObservation (const ProfilingLock & /*unused*/) noexcept
{
    ++observers;

    // If there are existing events, return last one to the observer, otherwise it will start reading old events.
    if (last)
    {
        ++last->observers;
        return last;
    }

    ++freshObservers;
    return nullptr;
}

const EventNode *EventManager::RequestNext (const EventNode *_current, const ProfilingLock & /*unused*/) noexcept
{
    if (_current)
    {
        assert (_current->observers != 0u);
        EventNode *next = _current->next;

        if (next)
        {
            --const_cast<EventNode *> (_current)->observers; // It was const only for event observer,
            ++next->observers;

            DropOutdatedEvents ();
            return next;
        }

        return nullptr;
    }

    if (first)
    {
        ++first->observers;
        --freshObservers;
    }

    return first;
}

void EventManager::FinishObservation (const EventNode *_current, const ProfilingLock & /*unused*/) noexcept
{
    assert (observers > 0u);
    --observers;

    if (_current)
    {
        --const_cast<EventNode *> (_current)->observers; // It was const only for event observer,
    }
    else
    {
        --freshObservers;
    }

    DropOutdatedEvents ();
}

void EventManager::RegisterNode (EventNode *_node) noexcept
{
    assert (_node);
    if (last)
    {
        last->next = _node;
        last = _node;
    }
    else
    {
        assert (!first);
        first = _node;
        last = _node;
    }
}

void EventManager::DropOutdatedEvents () noexcept
{
    if (freshObservers > 0u)
    {
        return;
    }

    while (first && first->observers == 0u)
    {
        EventNode *next = first->next;
        first->~EventNode ();
        events.Release (first);
        first = next;
    }

    if (!first)
    {
        last = nullptr;
    }
}
} // namespace Emergence::Memory::Profiler::Original

#include <Assert/Assert.hpp>

#include <Log/Log.hpp>

#include <Memory/Recording/Constants.hpp>
#include <Memory/Recording/Track.hpp>

namespace Emergence::Memory::Recording
{
RecordedAllocationGroup::Iterator &RecordedAllocationGroup::Iterator::operator++ () noexcept
{
    EMERGENCE_ASSERT (current);
    current = current->nextOnLevel.get ();
    return *this;
}

RecordedAllocationGroup::Iterator RecordedAllocationGroup::Iterator::operator++ (int) noexcept
{
    Iterator previous = *this;
    ++*this;
    return previous;
}

const RecordedAllocationGroup *RecordedAllocationGroup::Iterator::operator* () const noexcept
{
    return current;
}

RecordedAllocationGroup::Iterator::Iterator (const RecordedAllocationGroup::Iterator &_other) noexcept = default;

RecordedAllocationGroup::Iterator::Iterator (RecordedAllocationGroup::Iterator &&_other) noexcept = default;

RecordedAllocationGroup::Iterator &RecordedAllocationGroup::Iterator::operator= (
    const RecordedAllocationGroup::Iterator &_other) noexcept = default;

RecordedAllocationGroup::Iterator &RecordedAllocationGroup::Iterator::operator= (
    RecordedAllocationGroup::Iterator &&_other) noexcept = default;

RecordedAllocationGroup::Iterator::~Iterator () noexcept = default;

bool RecordedAllocationGroup::Iterator::operator== (const RecordedAllocationGroup::Iterator &_other) const noexcept
{
    return current == _other.current;
}

bool RecordedAllocationGroup::Iterator::operator!= (const RecordedAllocationGroup::Iterator &_other) const noexcept
{
    return !(*this == _other);
}

RecordedAllocationGroup::Iterator::Iterator (const RecordedAllocationGroup *_current) noexcept
    : current (_current)
{
}

const RecordedAllocationGroup *RecordedAllocationGroup::Parent () const noexcept
{
    return parent;
}

RecordedAllocationGroup::Iterator RecordedAllocationGroup::BeginChildren () const noexcept
{
    return Iterator {firstChild.get ()};
}

// NOLINTNEXTLINE(readability-convert-member-functions-to-static): Interface looks visually better without static.
RecordedAllocationGroup::Iterator RecordedAllocationGroup::EndChildren () const noexcept
{
    return Iterator {nullptr};
}

UniqueString RecordedAllocationGroup::GetId () const noexcept
{
    return id;
}

size_t RecordedAllocationGroup::GetAcquired () const noexcept
{
    return acquired;
}

size_t RecordedAllocationGroup::GetReserved () const noexcept
{
    return reserved;
}

size_t RecordedAllocationGroup::GetTotal () const noexcept
{
    return acquired + reserved;
}

UnorderedPool &RecordedAllocationGroup::GetGroupPool () noexcept
{
    static UnorderedPool pool {Constants::AllocationGroup (), sizeof (RecordedAllocationGroup),
                               alignof (RecordedAllocationGroup)};
    return pool;
}

void *RecordedAllocationGroup::operator new (std::size_t /*unused*/) noexcept
{
    return GetGroupPool ().Acquire ();
}

void RecordedAllocationGroup::operator delete (void *_pointer) noexcept
{
    GetGroupPool ().Release (_pointer);
}

RecordedAllocationGroup::RecordedAllocationGroup (RecordedAllocationGroup *_parent,
                                                  Memory::UniqueString _id,
                                                  std::size_t _reserved,
                                                  std::size_t _acquired) noexcept
    : id (_id),
      reserved (_reserved),
      acquired (_acquired),
      parent (_parent)
{
    if (parent)
    {
        nextOnLevel.reset (this);
        nextOnLevel.swap (parent->firstChild);
    }
}

void RecordedAllocationGroup::Allocate (std::uint64_t _bytes) noexcept
{
    if (parent)
    {
        parent->Allocate (_bytes);
    }

    reserved += _bytes;
}

bool RecordedAllocationGroup::Acquire (std::uint64_t _bytes) noexcept
{
    if (_bytes <= reserved && (!parent || parent->Acquire (_bytes)))
    {
        reserved -= _bytes;
        acquired += _bytes;
        return true;
    }

    return false;
}

bool RecordedAllocationGroup::Release (std::uint64_t _bytes) noexcept
{
    if (_bytes <= acquired && (!parent || parent->Release (_bytes)))
    {
        acquired -= _bytes;
        reserved += _bytes;
        return true;
    }

    return false;
}

bool RecordedAllocationGroup::Free (std::uint64_t _bytes) noexcept
{
    if (_bytes <= reserved && (!parent || parent->Free (_bytes)))
    {
        reserved -= _bytes;
        return true;
    }

    return false;
}

Track::EventIterator::EventIterator (const Track::EventIterator &_other) noexcept = default;

Track::EventIterator::EventIterator (Track::EventIterator &&_other) noexcept = default;

const Event *Track::EventIterator::operator* () const noexcept
{
    if (current)
    {
        return &current->event;
    }

    return nullptr;
}

Track::EventIterator &Track::EventIterator::operator= (const Track::EventIterator &_other) noexcept = default;

Track::EventIterator &Track::EventIterator::operator= (Track::EventIterator &&_other) noexcept = default;

Track::EventIterator::~EventIterator () noexcept = default;

Track::EventIterator &Track::EventIterator::operator++ () noexcept
{
    EMERGENCE_ASSERT (track);
    if (current)
    {
        current = current->next;
    }
    else
    {
        // Circle from the end to first element.
        current = track->first;
    }

    return *this;
}

Track::EventIterator Track::EventIterator::operator++ (int) noexcept
{
    EventIterator previous = *this;
    ++*this;
    return previous;
}

Track::EventIterator &Track::EventIterator::operator-- () noexcept
{
    EMERGENCE_ASSERT (track);
    if (current)
    {
        current = current->previous;
    }
    else
    {
        // Go from the end to last element.
        current = track->last;
    }

    return *this;
}

Track::EventIterator Track::EventIterator::operator-- (int) noexcept
{
    EventIterator previous = *this;
    --*this;
    return previous;
}

bool Track::EventIterator::operator== (const Track::EventIterator &_other) const noexcept
{
    // Comparing iterators from different recordings seems error-prone.
    EMERGENCE_ASSERT (track == _other.track);
    return current == _other.current;
}

bool Track::EventIterator::operator!= (const Track::EventIterator &_other) const noexcept
{
    return !(*this == _other);
}

Track::EventIterator::EventIterator (const Track *_track, const Track::EventNode *_current) noexcept
    : track (_track),
      current (_current)
{
}

Track::Track () noexcept
    : idToGroup (Constants::AllocationGroup ()),
      events (Constants::AllocationGroup (), sizeof (EventNode), alignof (EventNode))
{
}

const RecordedAllocationGroup *Track::Root () const noexcept
{
    return root.get ();
}

Track::EventIterator Track::EventBegin () const noexcept
{
    return EventIterator {this, first};
}

Track::EventIterator Track::EventCurrent () const noexcept
{
    return EventIterator {this, current};
}

Track::EventIterator Track::EventEnd () const noexcept
{
    return EventIterator {this, nullptr};
}

bool Track::MoveToPreviousEvent () noexcept
{
    if (!current)
    {
        return false;
    }

    switch (current->event.type)
    {
    case EventType::DECLARE_GROUP:
        if (!UndoDeclareGroupEvent (current->event))
        {
            return false;
        }

        break;
    case EventType::ALLOCATE:
        if (!UndoAllocateEvent (current->event))
        {
            return false;
        }

        break;
    case EventType::ACQUIRE:
        if (!UndoAcquireEvent (current->event))
        {
            return false;
        }

        break;
    case EventType::RELEASE:
        if (!UndoReleaseEvent (current->event))
        {
            return false;
        }

        break;
    case EventType::FREE:
        if (!UndoFreeEvent (current->event))
        {
            return false;
        }

        break;
    case EventType::MARKER:
        break;
    }

    current = current->previous;
    return true;
}

bool Track::MoveToNextEvent () noexcept
{
    if (current == last || !first)
    {
        return false;
    }

    EventNode *next = current ? current->next : first;
    EMERGENCE_ASSERT (next);

    switch (next->event.type)
    {
    case EventType::DECLARE_GROUP:
        if (!ApplyDeclareGroupEvent (next->event))
        {
            return false;
        }

        break;
    case EventType::ALLOCATE:
        if (!ApplyAllocateEvent (next->event))
        {
            return false;
        }

        break;
    case EventType::ACQUIRE:
        if (!ApplyAcquireEvent (next->event))
        {
            return false;
        }

        break;
    case EventType::RELEASE:
        if (!ApplyReleaseEvent (next->event))
        {
            return false;
        }

        break;
    case EventType::FREE:
        if (!ApplyFreeEvent (next->event))
        {
            return false;
        }

        break;
    case EventType::MARKER:
        break;
    }

    current = next;
    return true;
}

const RecordedAllocationGroup *Track::GetGroupByUID (GroupUID _uid) const noexcept
{
    if (_uid < idToGroup.size ())
    {
        return idToGroup[_uid];
    }

    return nullptr;
}

void Track::Clear () noexcept
{
    root.reset ();
    idToGroup.clear ();

    events.Clear ();
    first = nullptr;
    last = nullptr;
    current = nullptr;
}

void Track::ReportEvent (const Event &_event) noexcept
{
    auto *node = new (events.Acquire ()) EventNode {.event = _event};
    if (last)
    {
        if (_event.timeNs < last->event.timeNs)
        {
            EMERGENCE_LOG (ERROR, "Recording::Track: Unable to report event because it breaks time order!");
            events.Release (node);
            return;
        }

        last->next = node;
        node->previous = last;
        last = node;
    }
    else
    {
        EMERGENCE_ASSERT (!first);
        first = node;
        last = node;
    }
}

RecordedAllocationGroup *Track::RequireGroup (GroupUID _uid) const noexcept
{
    EMERGENCE_ASSERT (_uid != MISSING_GROUP_ID);
    RecordedAllocationGroup *group = _uid < idToGroup.size () ? idToGroup[_uid] : nullptr;

    if (!group)
    {
        EMERGENCE_LOG (ERROR, "Recording::Track: Unable to find group with uid ", _uid, "!");
    }

    return group;
}

bool Track::ApplyDeclareGroupEvent (const Event &_event) noexcept
{
    EMERGENCE_ASSERT (_event.type == EventType::DECLARE_GROUP);
    EMERGENCE_ASSERT (_event.uid != MISSING_GROUP_ID);

    if (_event.uid < idToGroup.size ())
    {
        auto *group = idToGroup[_event.uid];
        if (group->GetId () != _event.id)
        {
            EMERGENCE_LOG (ERROR, "Recording::Track: Two groups contend for one UID!");
            return false;
        }

        group->reserved = _event.reservedBytes;
        group->acquired = _event.acquiredBytes;
        return true;
    }

    if (_event.uid != idToGroup.size ())
    {
        EMERGENCE_LOG (ERROR, "Recording::Track: Group uids are expected to be incremental.");
        return false;
    }

    if (_event.parent == MISSING_GROUP_ID)
    {
        // Root group declaration.
        EMERGENCE_ASSERT (!*_event.id);
        EMERGENCE_ASSERT (!root); // If root exists, it should have been found by id previously.

        root.reset (new RecordedAllocationGroup {nullptr, {}, _event.reservedBytes, _event.acquiredBytes});
        EMERGENCE_ASSERT (idToGroup.empty ()); // If we created any groups before root, then our logic is broken.
        EMERGENCE_ASSERT (idToGroup.size () == _event.uid);
        idToGroup.emplace_back (root.get ());

        return true;
    }

    if (RecordedAllocationGroup *parent = RequireGroup (_event.parent))
    {
        // We do not need to check for overlapping ids in children here, because we have already checked uids.
        idToGroup.emplace_back (
            new RecordedAllocationGroup {parent, _event.id, _event.reservedBytes, _event.acquiredBytes});
        return true;
    }

    return false;
}

bool Track::ApplyAllocateEvent (const Event &_event) noexcept
{
    EMERGENCE_ASSERT (_event.type == EventType::ALLOCATE);
    if (RecordedAllocationGroup *group = RequireGroup (_event.group))
    {
        group->Allocate (_event.bytes);
        return true;
    }

    return false;
}

bool Track::ApplyAcquireEvent (const Event &_event) noexcept
{
    EMERGENCE_ASSERT (_event.type == EventType::ACQUIRE);
    if (RecordedAllocationGroup *group = RequireGroup (_event.group))
    {
        if (group->Acquire (_event.bytes))
        {
            return true;
        }

        EMERGENCE_LOG (ERROR,
                       "Recording::Track: Unable to apply acquire because of either broken state or broken replay!");
    }

    return false;
}

bool Track::ApplyReleaseEvent (const Event &_event) noexcept
{
    EMERGENCE_ASSERT (_event.type == EventType::RELEASE);
    if (RecordedAllocationGroup *group = RequireGroup (_event.group))
    {
        if (group->Release (_event.bytes))
        {
            return true;
        }

        EMERGENCE_LOG (ERROR,
                       "Recording::Track: Unable to apply release because of either broken state or broken replay!");
    }

    return false;
}

bool Track::ApplyFreeEvent (const Event &_event) noexcept
{
    EMERGENCE_ASSERT (_event.type == EventType::FREE);
    if (RecordedAllocationGroup *group = RequireGroup (_event.group))
    {
        if (group->Free (_event.bytes))
        {
            return true;
        }

        EMERGENCE_LOG (ERROR, "Recording::Track: Unable apply free because of either broken state or broken replay!");
    }

    return false;
}

bool Track::UndoDeclareGroupEvent (const Event &_event) noexcept
{
    EMERGENCE_ASSERT (_event.type == EventType::DECLARE_GROUP);
    if (_event.uid < idToGroup.size ())
    {
        auto *group = idToGroup[_event.uid];
        if (group->GetId () != _event.id)
        {
            EMERGENCE_LOG (ERROR, "Recording::Track: Two groups contend for one UID!");
            return false;
        }

        group->reserved = 0u;
        group->acquired = 0u;
        return true;
    }

    EMERGENCE_LOG (ERROR, "Recording::Track: Unable to locate group to undo its declaration!");
    return false;
}

bool Track::UndoAllocateEvent (const Event &_event) noexcept
{
    EMERGENCE_ASSERT (_event.type == EventType::ALLOCATE);
    if (RecordedAllocationGroup *group = RequireGroup (_event.group))
    {
        if (group->Free (_event.bytes))
        {
            return true;
        }

        EMERGENCE_LOG (ERROR,
                       "Recording::Track: Unable to undo allocate because of either broken state or broken replay!");
    }

    return false;
}

bool Track::UndoAcquireEvent (const Event &_event) noexcept
{
    EMERGENCE_ASSERT (_event.type == EventType::ACQUIRE);
    if (RecordedAllocationGroup *group = RequireGroup (_event.group))
    {
        if (group->Release (_event.bytes))
        {
            return true;
        }

        EMERGENCE_LOG (ERROR,
                       "Recording::Track: Unable to undo acquire because of either broken state or broken replay!");
    }

    return false;
}

bool Track::UndoReleaseEvent (const Event &_event) noexcept
{
    EMERGENCE_ASSERT (_event.type == EventType::RELEASE);
    if (RecordedAllocationGroup *group = RequireGroup (_event.group))
    {
        if (group->Acquire (_event.bytes))
        {
            return true;
        }

        EMERGENCE_LOG (ERROR,
                       "Recording::Track: Unable to undo release because of either broken state or broken replay!");
    }

    return false;
}

bool Track::UndoFreeEvent (const Event &_event) noexcept
{
    EMERGENCE_ASSERT (_event.type == EventType::FREE);
    if (RecordedAllocationGroup *group = RequireGroup (_event.group))
    {
        group->Allocate (_event.bytes);
        return true;
    }

    return false;
}
} // namespace Emergence::Memory::Recording

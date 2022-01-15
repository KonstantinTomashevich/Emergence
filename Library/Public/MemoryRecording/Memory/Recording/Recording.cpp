#include <cassert>

#include <Log/Log.hpp>

#include <Memory/Recording/Constants.hpp>
#include <Memory/Recording/Recording.hpp>

namespace Emergence::Memory::Recording
{
RecordedAllocationGroup::Iterator &RecordedAllocationGroup::Iterator::operator++ () noexcept
{
    assert (current);
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

RecordedAllocationGroup::Iterator::Iterator (const RecordedAllocationGroup *_current) noexcept : current (_current)
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

RecordedAllocationGroup::Iterator RecordedAllocationGroup::EndChildren () noexcept
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
    static UnorderedPool pool {Constants::AllocationGroup (), sizeof (RecordedAllocationGroup)};
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

Recording::EventIterator::EventIterator (const Recording::EventIterator &_other) noexcept = default;

Recording::EventIterator::EventIterator (Recording::EventIterator &&_other) noexcept = default;

const Event *Recording::EventIterator::operator* () const noexcept
{
    if (current)
    {
        return &current->event;
    }

    return nullptr;
}

Recording::EventIterator &Recording::EventIterator::operator= (const Recording::EventIterator &_other) noexcept =
    default;

Recording::EventIterator &Recording::EventIterator::operator= (Recording::EventIterator &&_other) noexcept = default;

Recording::EventIterator::~EventIterator () noexcept = default;

Recording::EventIterator &Recording::EventIterator::operator++ () noexcept
{
    assert (recording);
    assert (current);

    if (current)
    {
        current = current->next;
    }
    else
    {
        // Circle from the end to first element.
        current = recording->first;
    }

    return *this;
}

Recording::EventIterator Recording::EventIterator::operator++ (int) noexcept
{
    EventIterator previous = *this;
    ++*this;
    return previous;
}

Recording::EventIterator &Recording::EventIterator::operator-- () noexcept
{
    assert (recording);
    assert (current);

    if (current)
    {
        current = current->previous;
    }
    else
    {
        // Go from the end to last element.
        current = recording->last;
    }

    return *this;
}

Recording::EventIterator Recording::EventIterator::operator-- (int) noexcept
{
    EventIterator previous = *this;
    --*this;
    return previous;
}

bool Recording::EventIterator::operator== (const Recording::EventIterator &_other) const noexcept
{
    // Comparing iterators from different recordings seems error-prone.
    assert (recording == _other.recording);
    return current == _other.current;
}

bool Recording::EventIterator::operator!= (const Recording::EventIterator &_other) const noexcept
{
    return !(*this == _other);
}

Recording::EventIterator::EventIterator (const Recording *_recording, const Recording::EventNode *_current) noexcept
    : recording (_recording),
      current (_current)
{
}

Recording::Recording () noexcept
    : idToGroup (Constants::AllocationGroup ()),
      events (Constants::AllocationGroup (), sizeof (EventNode))
{
}

const RecordedAllocationGroup *Recording::Root () const noexcept
{
    return root.get ();
}

Recording::EventIterator Recording::EventBegin () const noexcept
{
    return EventIterator {this, first};
}

Recording::EventIterator Recording::EventCurrent () const noexcept
{
    return EventIterator {this, current};
}

Recording::EventIterator Recording::EventEnd () const noexcept
{
    return EventIterator {this, nullptr};
}

bool Recording::MoveToPreviousEvent () noexcept
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

bool Recording::MoveToNextEvent () noexcept
{
    if (current == last || !first)
    {
        return false;
    }

    EventNode *next = current ? current->next : first;
    assert (next);

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

void Recording::ReportEvent (const Event &_event) noexcept
{
    auto *node = new (events.Acquire ()) EventNode {.event = _event};
    if (last)
    {
        last->next = node;
        last = node;
    }
    else
    {
        assert (!first);
        first = node;
        last = node;
    }
}

RecordedAllocationGroup *Recording::RequireGroup (GroupUID _uid) const noexcept
{
    RecordedAllocationGroup *group = _uid < idToGroup.size () ? idToGroup[current->event.parent] : nullptr;
    if (!group)
    {
        Log::GlobalLogger::Log (Log::Level::ERROR,
                                "Recording: Unable to find group with uid " + Container::ToString (_uid) + "!");
    }

    return group;
}

bool Recording::ApplyDeclareGroupEvent (const Event &_event) noexcept
{
    assert (_event.type == EventType::DECLARE_GROUP);
    if (_event.parent == MISSING_GROUP_ID)
    {
        // Root group declaration.
        assert (!*_event.id);
        if (!root)
        {
            root.reset (new RecordedAllocationGroup {nullptr, {}, _event.reservedBytes, _event.acquiredBytes});
            assert (idToGroup.empty ()); // If we created any groups before root, then our logic is broken.
            idToGroup.emplace_back (root.get ());
        }
        else
        {
            root->reserved = _event.reservedBytes;
            root->acquired = _event.acquiredBytes;
        }

        return true;
    }

    if (RecordedAllocationGroup *parent = RequireGroup (_event.parent))
    {
        for (auto iterator = parent->BeginChildren (); iterator != parent->EndChildren (); ++iterator)
        {
            // Interface is const because it is available to user. Recording is allowed to change groups.
            auto *child = const_cast<RecordedAllocationGroup *> (*iterator);

            if (child->GetId () == _event.id)
            {
                child->reserved = _event.reservedBytes;
                child->acquired = _event.acquiredBytes;
                return true;
            }
        }

        idToGroup.emplace_back (
            new RecordedAllocationGroup {parent, _event.id, _event.reservedBytes, _event.acquiredBytes});
        return true;
    }

    return false;
}

bool Recording::ApplyAllocateEvent (const Event &_event) noexcept
{
    assert (_event.type == EventType::ALLOCATE);
    if (RecordedAllocationGroup *group = RequireGroup (_event.group))
    {
        group->reserved += _event.bytes;
        return true;
    }

    return false;
}

bool Recording::ApplyAcquireEvent (const Event &_event) noexcept
{
    assert (_event.type == EventType::ACQUIRE);
    if (RecordedAllocationGroup *group = RequireGroup (_event.group))
    {
        if (group->reserved >= _event.bytes)
        {
            group->acquired += _event.bytes;
            group->reserved -= _event.bytes;
            return true;
        }

        Log::GlobalLogger::Log (Log::Level::ERROR,
                                "Recording: Unable to apply acquire because of either broken state or broken replay!");
    }

    return false;
}

bool Recording::ApplyReleaseEvent (const Event &_event) noexcept
{
    assert (_event.type == EventType::RELEASE);
    if (RecordedAllocationGroup *group = RequireGroup (_event.group))
    {
        if (group->acquired >= _event.bytes)
        {
            group->reserved += _event.bytes;
            group->acquired -= _event.bytes;
            return true;
        }

        Log::GlobalLogger::Log (Log::Level::ERROR,
                                "Recording: Unable to apply release because of either broken state or broken replay!");
    }

    return false;
}

bool Recording::ApplyFreeEvent (const Event &_event) noexcept
{
    assert (_event.type == EventType::FREE);
    if (RecordedAllocationGroup *group = RequireGroup (_event.group))
    {
        if (group->reserved >= _event.bytes)
        {
            group->reserved -= _event.bytes;
            return true;
        }

        Log::GlobalLogger::Log (Log::Level::ERROR,
                                "Recording: Unable apply free because of either broken state or broken replay!");
    }

    return false;
}

bool Recording::UndoDeclareGroupEvent (const Event &_event) noexcept
{
    assert (_event.type == EventType::DECLARE_GROUP);
    if (_event.parent == MISSING_GROUP_ID)
    {
        // Root group declaration.
        assert (!*_event.id);
        if (root)
        {
            root->reserved = 0u;
            root->acquired = 0u;
            return true;
        }

        Log::GlobalLogger::Log (Log::Level::ERROR, "Recording: Unable to undo root creation.");
        return false;
    }

    if (RecordedAllocationGroup *parent = RequireGroup (_event.parent))
    {
        for (auto iterator = parent->BeginChildren (); iterator != parent->EndChildren (); ++iterator)
        {
            // Interface is const because it is available to user. Recording is allowed to change groups.
            auto *child = const_cast<RecordedAllocationGroup *> (*iterator);

            if (child->GetId () == _event.id)
            {
                child->reserved = 0u;
                child->acquired = 0u;
                return true;
            }
        }

        Log::GlobalLogger::Log (Log::Level::ERROR, "Recording: Unable to locate group to undo its declaration!");
        return false;
    }

    return false;
}

bool Recording::UndoAllocateEvent (const Event &_event) noexcept
{
    assert (_event.type == EventType::ALLOCATE);
    if (RecordedAllocationGroup *group = RequireGroup (_event.group))
    {
        if (group->reserved >= _event.bytes)
        {
            group->reserved -= _event.bytes;
            return true;
        }

        Log::GlobalLogger::Log (Log::Level::ERROR,
                                "Recording: Unable to undo allocate because of either broken state or broken replay!");
    }

    return false;
}

bool Recording::UndoAcquireEvent (const Event &_event) noexcept
{
    assert (_event.type == EventType::ACQUIRE);
    if (RecordedAllocationGroup *group = RequireGroup (_event.group))
    {
        if (group->acquired >= _event.bytes)
        {
            group->reserved += _event.bytes;
            group->acquired -= _event.bytes;
            return true;
        }

        Log::GlobalLogger::Log (Log::Level::ERROR,
                                "Recording: Unable to undo acquire because of either broken state or broken replay!");
    }

    return false;
}

bool Recording::UndoReleaseEvent (const Event &_event) noexcept
{
    assert (_event.type == EventType::RELEASE);
    if (RecordedAllocationGroup *group = RequireGroup (_event.group))
    {
        if (group->reserved >= _event.bytes)
        {
            group->acquired += _event.bytes;
            group->reserved -= _event.bytes;
            return true;
        }

        Log::GlobalLogger::Log (Log::Level::ERROR,
                                "Recording: Unable to undo release because of either broken state or broken replay!");
    }

    return false;
}

bool Recording::UndoFreeEvent (const Event &_event) noexcept
{
    assert (_event.type == EventType::FREE);
    if (RecordedAllocationGroup *group = RequireGroup (_event.group))
    {
        group->reserved += _event.bytes;
        return true;
    }

    return false;
}
} // namespace Emergence::Memory::Recording

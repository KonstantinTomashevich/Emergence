#include <Assert/Assert.hpp>

#include <Memory/Recording/ReportingHelpers.hpp>

namespace Emergence::Memory::Recording
{
GroupUID GroupUIDAssigner::GetUID (const Profiler::AllocationGroup &_group) const noexcept
{
    auto iterator = uids.find (_group);
    if (iterator != uids.end ())
    {
        return iterator->second;
    }

    return MISSING_GROUP_ID;
}

GroupUID GroupUIDAssigner::GetOrAssignUID (const Profiler::AllocationGroup &_group,
                                           const DeclarationConsumer &_declarationConsumer) noexcept
{
    if (_group == Profiler::AllocationGroup {})
    {
        return MISSING_GROUP_ID;
    }

    GroupUID alreadyAssigned = GetUID (_group);
    if (alreadyAssigned != MISSING_GROUP_ID)
    {
        return alreadyAssigned;
    }

    const GroupUID parent = GetOrAssignUID (_group.Parent (), _declarationConsumer);
    const GroupUID uid = counter++;
    EMERGENCE_ASSERT (uid != MISSING_GROUP_ID);
    uids[_group] = uid;

    _declarationConsumer ({
        0u /* Time should be known to user, therefore user is expected to set inside consumer. */,

        parent,
        _group.GetId (),
        uid,

        // Usually we create declare groups after encountering first event, associated with the group. If this group has
        // no uid, it means it wasn't captured (see ImportCapture), therefore it must be empty. Unfortunately, we can
        // not assert this expectation, because we can not get group state at the time of event creation.
        0u,
        0u,
    });

    return uid;
}

void GroupUIDAssigner::ImportCapture (const Profiler::CapturedAllocationGroup &_captured,
                                      const DeclarationConsumer &_declarationConsumer) noexcept
{
    GetOrAssignUID (_captured.GetSource (),
                    [&_captured, &_declarationConsumer] (Event _event)
                    {
                        // Nested assignment is disallowed for captured groups, because it makes
                        // no sense: ImportCapture should always be called on capture root group.
                        EMERGENCE_ASSERT (_event.id == _captured.GetId ());

                        _event.timeNs = _captured.GetCaptureTimeNs ();
                        _event.reservedBytes = _captured.GetReserved ();
                        _event.acquiredBytes = _captured.GetAcquired ();
                        _declarationConsumer (_event);
                    });

    for (auto iterator = _captured.BeginChildren (); iterator != _captured.EndChildren (); ++iterator)
    {
        ImportCapture (*iterator, _declarationConsumer);
    }
}

void GroupUIDAssigner::Clear () noexcept
{
    counter = 0u;
    uids.clear ();
}

Event ConvertEvent (GroupUID _groupUID, const Profiler::Event &_source) noexcept
{
    EMERGENCE_ASSERT (_groupUID != MISSING_GROUP_ID);
    switch (_source.type)
    {
    case Profiler::EventType::ALLOCATE:
        return {
            EventType::ALLOCATE,
            _source.timeNs,
            _groupUID,
            _source.bytes,
        };

    case Profiler::EventType::ACQUIRE:
        return {
            EventType::ACQUIRE,
            _source.timeNs,
            _groupUID,
            _source.bytes,
        };

    case Profiler::EventType::RELEASE:
        return {
            EventType::RELEASE,
            _source.timeNs,
            _groupUID,
            _source.bytes,
        };

    case Profiler::EventType::FREE:
        return {
            EventType::FREE,
            _source.timeNs,
            _groupUID,
            _source.bytes,
        };

    case Profiler::EventType::MARKER:
        return {
            _source.timeNs,
            _groupUID,
            _source.markerId,
        };
    }

    EMERGENCE_ASSERT (false);
    return {
        _source.timeNs,
        MISSING_GROUP_ID,
        // Should never happen, therefore we do not create unique string statically.
        UniqueString {"Unable to convert event of unknown type!"},
    };
}
} // namespace Emergence::Memory::Recording

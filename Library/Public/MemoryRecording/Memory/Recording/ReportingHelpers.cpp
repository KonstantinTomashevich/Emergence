#include <cassert>

#include <Memory/Recording/ReportingHelpers.hpp>

namespace Emergence::Memory::Recording
{
GroupUID GroupUIDAssigner::GetUID (const Profiler::AllocationGroup &_group) const noexcept
{
    auto iterator = ids.find (_group);
    if (iterator != ids.end ())
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
    ids[_group] = uid;

    _declarationConsumer (Event {
        .type = EventType::DECLARE_GROUP,
        .timeNs = 0u /* Time should be known to user. */,

        .parent = parent,
        .id = _group.GetId (),
        .uid = uid,

        // It is the first event, associated with this group, and this group wasn't captured, therefore it must be
        // empty. Unfortunately, we can not assert this statement, because we can not get group state at the time of
        // event creation.
        .reservedBytes = 0u,
        .acquiredBytes = 0u,
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
                        assert (_event.id == _captured.GetId ());

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
    ids.clear ();
}

Event ConvertEvent (GroupUID _groupUID, const Profiler::Event &_source) noexcept
{
    assert (_groupUID != MISSING_GROUP_ID);
    switch (_source.type)
    {
    case Profiler::EventType::ALLOCATE:
        return {
            .type = EventType::ALLOCATE,
            .timeNs = _source.timeNs,
            .group = _groupUID,
            .bytes = _source.bytes,
        };

    case Profiler::EventType::ACQUIRE:
        return {
            .type = EventType::ACQUIRE,
            .timeNs = _source.timeNs,
            .group = _groupUID,
            .bytes = _source.bytes,
        };

    case Profiler::EventType::RELEASE:
        return {
            .type = EventType::RELEASE,
            .timeNs = _source.timeNs,
            .group = _groupUID,
            .bytes = _source.bytes,
        };

    case Profiler::EventType::FREE:
        return {
            .type = EventType::FREE,
            .timeNs = _source.timeNs,
            .group = _groupUID,
            .bytes = _source.bytes,
        };

    case Profiler::EventType::MARKER:
        return {
            .type = EventType::MARKER,
            .timeNs = _source.timeNs,
            .scope = _groupUID,
            .markerId = _source.markerId,
        };
    }
}
} // namespace Emergence::Memory::Recording

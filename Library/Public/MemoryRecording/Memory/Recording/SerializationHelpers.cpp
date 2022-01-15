#include <cassert>

#include <Memory/Recording/SerializationHelpers.hpp>

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

    _declarationConsumer (_group, Event {
                                      .type = EventType::DECLARE_GROUP,
                                      .timeNs = 0u /* Time should be known to user. */,
                                      .reservedBytes = 0u /* Reserved bytes should be known to user. */,
                                      .acquiredBytes = 0u /* Acquired bytes should be known to user. */,
                                      .parent = parent,
                                      .id = _group.GetId (),
                                  });

    return uid;
}

void GroupUIDAssigner::ImportCapture (const Profiler::CapturedAllocationGroup &_captured,
                                      const DeclarationConsumer &_declarationConsumer) noexcept
{
    GetOrAssignUID (_captured.GetSource (),
                    [&_captured, &_declarationConsumer] (const Profiler::AllocationGroup &_group, Event _event)
                    {
                        // Nested assignment is disallowed for captured groups, because it makes
                        // no sense: ImportCapture should always be called on capture root group.
                        assert (_captured.GetSource () == _group);

                        _event.reservedBytes = _captured.GetReserved ();
                        _event.acquiredBytes = _captured.GetAcquired ();
                        _declarationConsumer (_group, _event);
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

Event ConvertEvent (GroupUID _predictedUid, const Profiler::Event &_source) noexcept
{
    assert (_predictedUid != MISSING_GROUP_ID);
    switch (_source.type)
    {
    case Profiler::EventType::ALLOCATE:
        return {
            .type = EventType::ALLOCATE,
            .timeNs = _source.timeNs,
            .group = _predictedUid,
            .bytes = _source.bytes,
        };

    case Profiler::EventType::ACQUIRE:
        return {
            .type = EventType::ACQUIRE,
            .timeNs = _source.timeNs,
            .group = _predictedUid,
            .bytes = _source.bytes,
        };

    case Profiler::EventType::RELEASE:
        return {
            .type = EventType::RELEASE,
            .timeNs = _source.timeNs,
            .group = _predictedUid,
            .bytes = _source.bytes,
        };

    case Profiler::EventType::FREE:
        return {
            .type = EventType::FREE,
            .timeNs = _source.timeNs,
            .group = _predictedUid,
            .bytes = _source.bytes,
        };

    case Profiler::EventType::MARKER:
        return {
            .type = EventType::MARKER,
            .timeNs = _source.timeNs,
            .scope = _predictedUid,
            .markerId = _source.markerId,
        };
    }
}
} // namespace Emergence::Memory::Recording

#include <cassert>

#include <Memory/Recording/RuntimeReporter.hpp>

namespace Emergence::Memory::Recording
{
RuntimeReporter::RuntimeReporter (RuntimeReporter &&_other) noexcept : uidAssigner (std::move (_other.uidAssigner))
{
    _other.uidAssigner.Clear ();
}

RuntimeReporter::~RuntimeReporter () noexcept
{
    End ();
}

void RuntimeReporter::Begin (Recording *_target, const Profiler::CapturedAllocationGroup &_capturedRoot) noexcept
{
    DeserializerBase::Begin (_target);
    uidAssigner.ImportCapture (_capturedRoot,
                               [this] (const Profiler::AllocationGroup & /*unused*/, Event _event)
                               {
                                   // For simplicity, initial capture data is stored as startup-time events.
                                   _event.timeNs = 0u;
                                   DeserializerBase::ReportEvent (_event);
                               });

    DeserializerBase::ReportEvent (Event {
        .type = EventType::MARKER,
        .timeNs = 0u,
        .scope = uidAssigner.GetUID (Profiler::AllocationGroup::Root ()),
        .markerId = Constants::RecordingInitializationFinishedMarker (),
    });
}

void RuntimeReporter::ReportEvent (const Profiler::Event &_event) noexcept
{
    // We can not report events from our group, because it will lead to infinite event generation.
    if (_event.group == Constants::AllocationGroup ())
    {
        return;
    }

    GroupUID uid = uidAssigner.GetOrAssignUID (
        _event.group,
        [this, &_event] (const Profiler::AllocationGroup & /*unused*/, Event _declarationEvent)
        {
            _declarationEvent.timeNs = _event.timeNs;

            // It is the first event, connected to this group, and this group
            // wasn't captured, therefore it must be empty. Unfortunately, we can
            // not assert this, because we can not get group data at the time of
            // event creation.
            _declarationEvent.reservedBytes = 0u;
            _declarationEvent.acquiredBytes = 0u;

            DeserializerBase::ReportEvent (_declarationEvent);
        });

    assert (uid != MISSING_GROUP_ID);
    DeserializerBase::ReportEvent (ConvertEvent (uid, _event));
}

void RuntimeReporter::End () noexcept
{
    uidAssigner.Clear ();
    DeserializerBase::End ();
}

RuntimeReporter &RuntimeReporter::operator= (RuntimeReporter &&_other) noexcept
{
    if (this != &_other)
    {
        this->~RuntimeReporter ();
        new (this) RuntimeReporter (std::move (_other));
    }

    return *this;
}
} // namespace Emergence::Memory::Recording

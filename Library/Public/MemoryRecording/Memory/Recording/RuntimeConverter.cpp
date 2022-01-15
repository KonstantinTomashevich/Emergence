#include <cassert>

#include <Memory/Recording/RuntimeConverter.hpp>

namespace Emergence::Memory::Recording
{
RuntimeConverter::RuntimeConverter (RuntimeConverter &&_other) noexcept
    : observer (std::move (_other.observer)),
      uidAssigner (std::move (_other.uidAssigner))
{
    _other.observer.reset ();
    _other.uidAssigner.Clear ();
}

RuntimeConverter::~RuntimeConverter () noexcept
{
    End ();
}

void RuntimeConverter::Begin (Recording *_target) noexcept
{
    DeserializerBase::Begin (_target);
    auto [capturedRoot, eventObserver] = Profiler::Capture::Start ();
    observer.emplace (std::move (eventObserver));

    uidAssigner.ImportCapture (capturedRoot,
                               [this] (const Profiler::AllocationGroup & /*unused*/, Event _event)
                               {
                                   // For simplicity, initial capture data is stored as startup-time events.
                                   _event.timeNs = 0u;
                                   ReportEvent (_event);
                               });

    ReportEvent (Event {
        .type = EventType::MARKER,
        .timeNs = 0u,
        .scope = uidAssigner.GetUID (Profiler::AllocationGroup::Root ()),
        .markerId = Constants::RecordingInitializationFinishedMarker (),
    });
}

bool RuntimeConverter::TryReadNext () noexcept
{
    assert (observer.has_value ());
    while (const Profiler::Event *next = observer->NextEvent ())
    {
        GroupUID uid =
            uidAssigner.GetOrAssignUID (next->group,
                                        [this, next] (const Profiler::AllocationGroup & /*unused*/, Event _event)
                                        {
                                            _event.timeNs = next->timeNs;

                                            // It is the first event, connected to this group, and this group wasn't
                                            // captured, therefore it must be empty. Unfortunately, we can not assert
                                            // this, because we can not get group data at the time of event creation.
                                            _event.reservedBytes = 0u;
                                            _event.acquiredBytes = 0u;

                                            ReportEvent (_event);
                                        });

        assert (uid != MISSING_GROUP_ID);
        ReportEvent (ConvertEvent (uid, *next));
        return true;
    }

    return false;
}

void RuntimeConverter::End () noexcept
{
    observer.reset ();
    uidAssigner.Clear ();
    DeserializerBase::End ();
}

RuntimeConverter &RuntimeConverter::operator= (RuntimeConverter &&_other) noexcept
{
    if (this != &_other)
    {
        this->~RuntimeConverter ();
        new (this) RuntimeConverter (std::move (_other));
    }

    return *this;
}
} // namespace Emergence::Memory::Recording

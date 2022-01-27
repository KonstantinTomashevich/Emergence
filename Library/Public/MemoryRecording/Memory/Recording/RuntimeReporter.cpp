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

void RuntimeReporter::Begin (Track *_target, const Profiler::CapturedAllocationGroup &_capturedRoot) noexcept
{
    ReporterBase::Begin (_target);
    uidAssigner.ImportCapture (_capturedRoot,
                               [this] (Event _event)
                               {
                                   ReporterBase::ReportEvent (_event);
                               });

    ReporterBase::ReportEvent ({
        _capturedRoot.GetCaptureTimeNs (),
        uidAssigner.GetUID (Profiler::AllocationGroup::Root ()),
        Constants::CaptureInitializationFinishedMarker (),
    });
}

void RuntimeReporter::ReportEvent (const Profiler::Event &_event) noexcept
{
    // We can not report events from our group, because it will lead to infinite event generation.
    if (_event.group == Constants::AllocationGroup ())
    {
        return;
    }

    GroupUID uid = uidAssigner.GetOrAssignUID (_event.group,
                                               [this, &_event] (Event _declarationEvent)
                                               {
                                                   _declarationEvent.timeNs = _event.timeNs;
                                                   ReporterBase::ReportEvent (_declarationEvent);
                                               });

    ReporterBase::ReportEvent (ConvertEvent (uid, _event));
}

void RuntimeReporter::End () noexcept
{
    uidAssigner.Clear ();
    ReporterBase::End ();
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

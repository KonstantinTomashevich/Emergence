#define _CRT_SECURE_NO_WARNINGS

#include <cassert>

#include <Memory/Recording/StreamSerializer.hpp>

#include <Serialization/Binary.hpp>

namespace Emergence::Memory::Recording
{
StreamSerializer::StreamSerializer (StreamSerializer &&_other) noexcept
    : output (_other.output),
      uidAssigner (std::move (_other.uidAssigner))
{
    _other.output = nullptr;
    _other.uidAssigner.Clear ();
}

StreamSerializer::~StreamSerializer () noexcept
{
    End ();
}

void StreamSerializer::Begin (std::ostream *_output, const Profiler::CapturedAllocationGroup &_capturedRoot) noexcept
{
    assert (_output);
    output = _output;

    uidAssigner.ImportCapture (_capturedRoot,
                               [this] (Event _event)
                               {
                                   Serialization::Binary::SerializeObject (*output, &_event, Event::Reflect ().mapping);
                               });

    const Event event {
        _capturedRoot.GetCaptureTimeNs (),
        uidAssigner.GetUID (Profiler::AllocationGroup::Root ()),
        Constants::CaptureInitializationFinishedMarker (),
    };

    Serialization::Binary::SerializeObject (*output, &event, Event::Reflect ().mapping);
}

void StreamSerializer::SerializeEvent (const Profiler::Event &_event) noexcept
{
    GroupUID uid = uidAssigner.GetOrAssignUID (_event.group,
                                               [this, &_event] (Event _declarationEvent)
                                               {
                                                   _declarationEvent.timeNs = _event.timeNs;
                                                   Serialization::Binary::SerializeObject (*output, &_declarationEvent,
                                                                                           Event::Reflect ().mapping);
                                               });

    const Event event = ConvertEvent (uid, _event);
    Serialization::Binary::SerializeObject (*output, &event, Event::Reflect ().mapping);
}

void StreamSerializer::End () noexcept
{
    output = nullptr;
    uidAssigner.Clear ();
}

StreamSerializer &StreamSerializer::operator= (StreamSerializer &&_other) noexcept
{
    if (this != &_other)
    {
        this->~StreamSerializer ();
        new (this) StreamSerializer (std::move (_other));
    }

    return *this;
}
} // namespace Emergence::Memory::Recording

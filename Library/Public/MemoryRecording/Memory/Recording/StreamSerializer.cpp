#define _CRT_SECURE_NO_WARNINGS

#include <cassert>

#include <Memory/Recording/StreamSerializer.hpp>

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
                               [this] (const Profiler::AllocationGroup & /*unused*/, Event _event)
                               {
                                   // For simplicity, initial capture data is stored as startup-time events.
                                   _event.timeNs = 0u;
                                   WriteEvent (_event);
                               });

    WriteEvent (Event {
        .type = EventType::MARKER,
        .timeNs = 0u,
        .scope = uidAssigner.GetUID (Profiler::AllocationGroup::Root ()),
        .markerId = Constants::RecordingInitializationFinishedMarker (),
    });
}

void StreamSerializer::SerializeEvent (const Profiler::Event &_event) noexcept
{
    GroupUID uid = uidAssigner.GetOrAssignUID (
        _event.group,
        [this, &_event] (const Profiler::AllocationGroup & /*unused*/, Event _declarationEvent)
        {
            _declarationEvent.timeNs = _event.timeNs;

            // It is the first event, connected to this group, and this group wasn't
            // captured, therefore it must be empty. Unfortunately, we can not assert
            // this, because we can not get group data at the time of event creation.
            _declarationEvent.reservedBytes = 0u;
            _declarationEvent.acquiredBytes = 0u;

            WriteEvent (_declarationEvent);
        });

    assert (uid != MISSING_GROUP_ID);
    WriteEvent (ConvertEvent (uid, _event));
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

void StreamSerializer::WriteEvent (const Event &_event) noexcept
{
    assert (output);

    // TODO: Implement generic serialization library, based on StandardLayoutMapping?

    output->write (reinterpret_cast<const char *> (&_event.type), sizeof (_event.type));
    output->write (reinterpret_cast<const char *> (&_event.timeNs), sizeof (_event.timeNs));

    auto writeUniqueString = [this] (UniqueString _string)
    {
        const char *symbols = *_string;
        if (symbols)
        {
            output->write (symbols, static_cast<std::streamsize> (strlen (symbols) + 1u));
        }
        else
        {
            output->put ('\0');
        }
    };

    // TODO: We are doing a lot of small writes/reads, which could lead to performance problems. Think about it later.

    switch (_event.type)
    {
    case EventType::DECLARE_GROUP:
        output->write (reinterpret_cast<const char *> (&_event.reservedBytes), sizeof (_event.reservedBytes));
        output->write (reinterpret_cast<const char *> (&_event.acquiredBytes), sizeof (_event.acquiredBytes));
        output->write (reinterpret_cast<const char *> (&_event.parent), sizeof (_event.parent));
        writeUniqueString (_event.id);
        break;

    case EventType::ALLOCATE:
    case EventType::ACQUIRE:
    case EventType::RELEASE:
    case EventType::FREE:
        output->write (reinterpret_cast<const char *> (&_event.group), sizeof (_event.group));
        output->write (reinterpret_cast<const char *> (&_event.bytes), sizeof (_event.bytes));
        break;

    case EventType::MARKER:
        output->write (reinterpret_cast<const char *> (&_event.scope), sizeof (_event.scope));
        writeUniqueString (_event.markerId);
        break;
    }
}
} // namespace Emergence::Memory::Recording

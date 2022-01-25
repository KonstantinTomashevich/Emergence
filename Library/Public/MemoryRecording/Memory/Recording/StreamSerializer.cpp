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
                               [this] (Event _event)
                               {
                                   WriteEvent (_event);
                               });

    WriteEvent ({
        _capturedRoot.GetCaptureTimeNs (),
        uidAssigner.GetUID (Profiler::AllocationGroup::Root ()),
        Constants::CaptureInitializationFinishedMarker (),
    });
}

void StreamSerializer::SerializeEvent (const Profiler::Event &_event) noexcept
{
    GroupUID uid = uidAssigner.GetOrAssignUID (_event.group,
                                               [this, &_event] (Event _declarationEvent)
                                               {
                                                   _declarationEvent.timeNs = _event.timeNs;
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
        output->write (reinterpret_cast<const char *> (&_event.parent), sizeof (_event.parent));
        writeUniqueString (_event.id);
        output->write (reinterpret_cast<const char *> (&_event.uid), sizeof (_event.uid));
        output->write (reinterpret_cast<const char *> (&_event.reservedBytes), sizeof (_event.reservedBytes));
        output->write (reinterpret_cast<const char *> (&_event.acquiredBytes), sizeof (_event.acquiredBytes));
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

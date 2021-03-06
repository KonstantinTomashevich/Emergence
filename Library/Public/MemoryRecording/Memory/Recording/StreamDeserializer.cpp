#include <cassert>

#include <Log/Log.hpp>

#include <Memory/Recording/StreamDeserializer.hpp>

namespace Emergence::Memory::Recording
{
StreamDeserializer::StreamDeserializer (StreamDeserializer &&_other) noexcept
    : ReporterBase (std::move (_other)),
      input (_other.input)
{
    _other.input = nullptr;
}

StreamDeserializer::~StreamDeserializer () noexcept
{
    End ();
}

void StreamDeserializer::Begin (Track *_target, std::istream *_input) noexcept
{
    assert (_input);
    ReporterBase::Begin (_target);
    input = _input;
}

bool StreamDeserializer::TryReadNext () noexcept
{
    assert (input);
    if (!*input)
    {
        return false;
    }

    EventType eventType;
    if (!input->read (reinterpret_cast<char *> (&eventType), sizeof (eventType)))
    {
        return false;
    }

    std::uint64_t timeNs;
    if (!input->read (reinterpret_cast<char *> (&timeNs), sizeof (timeNs)))
    {
        return false;
    }

    switch (eventType)
    {
    case EventType::DECLARE_GROUP:
        return ReadDeclareGroupEvent (timeNs);

    case EventType::ALLOCATE:
    case EventType::ACQUIRE:
    case EventType::RELEASE:
    case EventType::FREE:
        return ReadMemoryManagementEvent (eventType, timeNs);

    case EventType::MARKER:
        return ReadMarkerEvent (timeNs);

    default:
        Log::GlobalLogger::Log (Log::Level::ERROR,
                                "StreamDeserializer: Unknown event type found, input might be broken.");
        return false;
    }
}

void StreamDeserializer::End () noexcept
{
    ReporterBase::End ();
}

StreamDeserializer &StreamDeserializer::operator= (StreamDeserializer &&_other) noexcept
{
    if (this != &_other)
    {
        this->~StreamDeserializer ();
        new (this) StreamDeserializer (std::move (_other));
    }

    return *this;
}

bool StreamDeserializer::ReadDeclareGroupEvent (std::uint64_t _timeNs) noexcept
{
    GroupUID parent;
    if (!input->read (reinterpret_cast<char *> (&parent), sizeof (parent)))
    {
        return false;
    }

    UniqueString id;
    if (!ReadUniqueString (id))
    {
        return false;
    }

    GroupUID uid;
    if (!input->read (reinterpret_cast<char *> (&uid), sizeof (uid)))
    {
        return false;
    }

    std::uint64_t reservedBytes;
    if (!input->read (reinterpret_cast<char *> (&reservedBytes), sizeof (reservedBytes)))
    {
        return false;
    }

    std::uint64_t acquiredBytes;
    if (!input->read (reinterpret_cast<char *> (&acquiredBytes), sizeof (acquiredBytes)))
    {
        return false;
    }

    ReportEvent ({
        _timeNs,
        parent,
        id,
        uid,
        reservedBytes,
        acquiredBytes,
    });

    return true;
}

bool StreamDeserializer::ReadMemoryManagementEvent (EventType _eventType, std::uint64_t _timeNs) noexcept
{
    GroupUID group;
    if (!input->read (reinterpret_cast<char *> (&group), sizeof (group)))
    {
        return false;
    }

    std::uint64_t bytes;
    if (!input->read (reinterpret_cast<char *> (&bytes), sizeof (bytes)))
    {
        return false;
    }

    ReportEvent ({
        _eventType,
        _timeNs,
        group,
        bytes,
    });

    return true;
}

bool StreamDeserializer::ReadMarkerEvent (std::uint64_t _timeNs) noexcept
{
    GroupUID scope;
    if (!input->read (reinterpret_cast<char *> (&scope), sizeof (scope)))
    {
        return false;
    }

    UniqueString markerId;
    if (!ReadUniqueString (markerId))
    {
        return false;
    }

    ReportEvent ({
        _timeNs,
        scope,
        markerId,
    });

    return true;
}

bool StreamDeserializer::ReadUniqueString (UniqueString &_output) noexcept
{
    Container::String string;
    while (true)
    {
        char next;
        if (!input->get (next))
        {
            return false;
        }

        if (next == '\0')
        {
            break;
        }

        string += next;
    }

    _output = UniqueString {string};
    return true;
}
} // namespace Emergence::Memory::Recording

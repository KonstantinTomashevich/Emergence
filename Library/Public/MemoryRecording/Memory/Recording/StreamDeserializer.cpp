#include <cassert>

#include <Log/Log.hpp>

#include <Memory/Recording/StreamDeserializer.hpp>

#include <Serialization/Binary.hpp>

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

    Event event {0u, 0u, UniqueString {}};
    if (Serialization::Binary::DeserializeObject (*input, &event, Event::Reflect ().mapping))
    {
        switch (event.type)
        {
        case EventType::DECLARE_GROUP:
        case EventType::ALLOCATE:
        case EventType::ACQUIRE:
        case EventType::RELEASE:
        case EventType::FREE:
        case EventType::MARKER:
            break;

        default:
            EMERGENCE_LOG (ERROR, "StreamDeserializer: Unknown event type found, input might be broken.");
            return false;
        }

        ReportEvent (event);
        return true;
    }

    return false;
}

void StreamDeserializer::End () noexcept
{
    ReporterBase::End ();
}
} // namespace Emergence::Memory::Recording

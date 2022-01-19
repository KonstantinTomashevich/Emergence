#include <sstream>

#include <Memory/Recording/RuntimeReporter.hpp>
#include <Memory/Recording/StreamDeserializer.hpp>
#include <Memory/Recording/StreamSerializer.hpp>
#include <Memory/Recording/Track.hpp>

#include <Testing/Testing.hpp>

using namespace Emergence::Memory;
using namespace Emergence::Memory::Literals;

static void CheckSerializationDeserializationRoutine (Recording::StreamSerializer &_serializer,
                                                      Recording::StreamDeserializer &_deserializer)
{
    // For simplicity, we use string stream as input/output buffer.
    // It might be not the best solution, but it should be OK for testing.
    std::stringstream buffer;

    Recording::Track runtimeTrack;
    Recording::Track deserializedTrack;
    auto [capturedRoot, observer] = Profiler::Capture::Start ();

    Recording::RuntimeReporter reporter;
    reporter.Begin (&runtimeTrack, capturedRoot);
    _serializer.Begin (&buffer, capturedRoot);

    Profiler::AllocationGroup additionalGroup {"CheckSerializationDeserializationRoutine"_us};
    additionalGroup.Allocate (2000u);
    additionalGroup.Acquire (1900u);
    additionalGroup.Release (1000u);
    additionalGroup.Acquire (500u);
    additionalGroup.Free (600u);

    while (const Profiler::Event *sourceEvent = observer.NextEvent ())
    {
        reporter.ReportEvent (*sourceEvent);

        // We do not serialize recording memory events to simplify testing, because these events
        // will not appear in runtime track (otherwise they would infinitely spawn new events).
        if (sourceEvent->group != Recording::Constants::AllocationGroup ())
        {
            _serializer.SerializeEvent (*sourceEvent);
        }
    }

    _serializer.End ();
    _deserializer.Begin (&deserializedTrack, &buffer);

    // Deserialize all events.
    while (_deserializer.TryReadNext ())
    {
    };

    // Compare events in runtime track and in deserialized track.
    auto runtimeIterator = runtimeTrack.EventBegin ();
    auto deserializedIterator = deserializedTrack.EventBegin ();

    while (runtimeIterator != runtimeTrack.EventEnd () && deserializedIterator != deserializedTrack.EventEnd ())
    {
        const Recording::Event *runtime = *runtimeIterator;
        const Recording::Event *deserialized = *deserializedIterator;

        CHECK_EQUAL (runtime->type, deserialized->type);
        CHECK_EQUAL (runtime->timeNs, deserialized->timeNs);

        switch (runtime->type)
        {
        case Recording::EventType::DECLARE_GROUP:
        {
            CHECK_EQUAL (runtime->reservedBytes, deserialized->reservedBytes);
            CHECK_EQUAL (runtime->acquiredBytes, deserialized->acquiredBytes);
            CHECK_EQUAL (runtime->parent, deserialized->parent);
            CHECK_EQUAL (runtime->id, deserialized->id);
            break;
        }
        case Recording::EventType::ALLOCATE:
        case Recording::EventType::ACQUIRE:
        case Recording::EventType::RELEASE:
        case Recording::EventType::FREE:
        {
            CHECK_EQUAL (runtime->group, deserialized->group);
            CHECK_EQUAL (runtime->bytes, deserialized->bytes);
            break;
        }
        case Recording::EventType::MARKER:
        {
            CHECK_EQUAL (runtime->scope, deserialized->scope);
            CHECK_EQUAL (runtime->markerId, deserialized->markerId);
            break;
        }
        }

        ++runtimeIterator;
        ++deserializedIterator;
    }

    CHECK_EQUAL (runtimeIterator, runtimeTrack.EventEnd ());
    CHECK_EQUAL (deserializedIterator, deserializedTrack.EventEnd ());
    _deserializer.End ();
}

BEGIN_SUITE (SerializationDeserialization)

TEST_CASE (EventComparison)
{
    Recording::StreamSerializer serializer;
    Recording::StreamDeserializer deserializer;
    CheckSerializationDeserializationRoutine (serializer, deserializer);

    Profiler::AllocationGroup additionalGroup {"EventComparison"_us};
    additionalGroup.Allocate (1000u);
    additionalGroup.Acquire (500u);

    // We execute check 2 times to ensure that serializer and deserializer are reusable.
    CheckSerializationDeserializationRoutine (serializer, deserializer);
}

TEST_CASE (CorruptedInput)
{
    Recording::Track track;
    auto passCorruptedInput = [&track] (const std::string &_input)
    {
        std::stringstream buffer {_input};
        Recording::StreamDeserializer deserializer;
        deserializer.Begin (&track, &buffer);
        CHECK (!deserializer.TryReadNext ());
    };

    passCorruptedInput ("Hello, world! Even more characters here..."); // Unknown event type.
    passCorruptedInput ("\1__________");                               // Not enough bytes to read whole event.
    passCorruptedInput ("\2_______");                                  // Not enough bytes to read whole event.
    passCorruptedInput ("\3Hello, world!");                            // Not enough bytes to read whole event.
    passCorruptedInput ("\4ooooooooffffffffxx");                       // Not enough bytes to read whole event.
    passCorruptedInput ("\5Hello, again! More and more and more characters here..."); // UniqueString parsing error.
}

END_SUITE

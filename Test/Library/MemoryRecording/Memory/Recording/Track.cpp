#include <Memory/Recording/RuntimeReporter.hpp>
#include <Memory/Recording/Track.hpp>

#include <SyntaxSugar/Time.hpp>

#include <Testing/Testing.hpp>

using namespace Emergence::Memory;
using namespace Emergence::Memory::Literals;

// Due to implementation specifics of MemoryRecording library, recorded groups order is reversed.
// For practical usage it doesn't matter, but in tests we are forced to use this not very beautiful helper.
template <typename OtherGroupType, typename Functor>
void CheckIteratorsWithReverseOrder (const Recording::RecordedAllocationGroup &_recorded,
                                     const OtherGroupType &_other,
                                     Functor _functor)
{
    auto count = [] (auto _begin, auto _end)
    {
        std::size_t count = 0u;
        auto iterator = _begin;

        while (iterator != _end)
        {
            ++iterator;
            ++count;
        }

        return count;
    };

    const std::size_t recordedCount = count (_recorded.BeginChildren (), _recorded.EndChildren ());
    const std::size_t otherCount = count (_other.BeginChildren (), _other.EndChildren ());
    CHECK_EQUAL (recordedCount, otherCount);

    if (recordedCount != otherCount)
    {
        return;
    }

    auto recordedIterator = _recorded.BeginChildren ();
    std::size_t index = 0u;

    while (recordedIterator != _recorded.EndChildren ())
    {
        auto otherIterator = _other.BeginChildren ();
        std::size_t steps = otherCount - index - 1u;

        while (steps > 0u)
        {
            ++otherIterator;
            --steps;
        }

        _functor (**recordedIterator, *otherIterator);
        ++recordedIterator;
        ++index;
    }
}

static void CompareWithCapture (const Recording::RecordedAllocationGroup &_recorded,
                                const Profiler::CapturedAllocationGroup &_captured)
{
    CHECK_EQUAL (_recorded.GetId (), _captured.GetId ());
    CHECK_EQUAL (_recorded.GetReserved (), _captured.GetReserved ());
    CHECK_EQUAL (_recorded.GetAcquired (), _captured.GetAcquired ());
    CHECK_EQUAL (_recorded.GetTotal (), _captured.GetTotal ());

    CheckIteratorsWithReverseOrder (_recorded, _captured,
                                    [&_recorded] (const Recording::RecordedAllocationGroup &_recordedChild,
                                                  const Profiler::CapturedAllocationGroup &_capturedChild)
                                    {
                                        CHECK_EQUAL (&_recorded, _recordedChild.Parent ());
                                        CompareWithCapture (_recordedChild, _capturedChild);
                                    });
}

static void CheckZeroUsage (const Recording::RecordedAllocationGroup &_recorded)
{
    CHECK_EQUAL (_recorded.GetReserved (), 0u);
    CHECK_EQUAL (_recorded.GetAcquired (), 0u);
    CHECK_EQUAL (_recorded.GetTotal (), 0u);

    for (auto iterator = _recorded.BeginChildren (); iterator != _recorded.EndChildren (); ++iterator)
    {
        CheckZeroUsage (**iterator);
    }
}

static bool IsAtInitializationEndMarker (const Recording::Track &_track)
{
    if (const Recording::Event *event = *_track.EventCurrent ())
    {
        return event->type == Recording::EventType::MARKER &&
               event->markerId == Recording::Constants::CaptureInitializationFinishedMarker ();
    }

    return false;
}

// Check if given profiler group is source of recorded group by comparing ids from root-to-group path.
static bool IsSource (const Profiler::AllocationGroup &_source, const Recording::RecordedAllocationGroup &_recorded)
{
    Profiler::AllocationGroup profilerGroup = _source;
    const Recording::RecordedAllocationGroup *recordedGroup = &_recorded;

    while (recordedGroup)
    {
        if (recordedGroup->GetId () != profilerGroup.GetId ())
        {
            return false;
        }

        recordedGroup = recordedGroup->Parent ();
        profilerGroup = profilerGroup.Parent ();
    }

    // Check that we climbed up to profiler groups root.
    return profilerGroup == Profiler::AllocationGroup {};
}

BEGIN_SUITE (Track)

TEST_CASE (EmptyTrack)
{
    Recording::Track track;
    CHECK_EQUAL (track.Root (), nullptr);
    CHECK_EQUAL (track.EventBegin (), track.EventEnd ());
    CHECK_EQUAL (track.EventCurrent (), track.EventEnd ());

    CHECK (!track.MoveToNextEvent ());
    CHECK (!track.MoveToPreviousEvent ());
}

TEST_CASE (CaptureInitialization)
{
    Profiler::AllocationGroup testGroup {"CaptureInitialization"_us};
    testGroup.Allocate (300u);
    testGroup.Acquire (100u);

    Recording::Track track;
    Profiler::CapturedAllocationGroup capturedRoot = Profiler::Capture::Start ().first;

    Recording::RuntimeReporter reporter;
    reporter.Begin (&track, capturedRoot);

    // Go to the last event, where all groups are initialized.
    while (!IsAtInitializationEndMarker (track))
    {
        REQUIRE_WITH_MESSAGE (track.MoveToNextEvent (), "Moving to initialization end marker.");
    }

    REQUIRE (track.Root ());
    CompareWithCapture (*track.Root (), capturedRoot);

    // Fallback to the beginning to check undo consistency.
    while (track.EventBegin () != track.EventCurrent ())
    {
        REQUIRE_WITH_MESSAGE (track.MoveToPreviousEvent (), "Falling back to the initial state.");
    }

    REQUIRE_WITH_MESSAGE (track.MoveToPreviousEvent (), "Falling back to the initial state.");
    REQUIRE (track.Root ());
    CheckZeroUsage (*track.Root ());

    // Go to the last event again to check how group declaration apply-undo-apply routine works.
    while (!IsAtInitializationEndMarker (track))
    {
        REQUIRE_WITH_MESSAGE (track.MoveToNextEvent (), "Moving to initialization end marker.");
    }

    REQUIRE (track.Root ());
    CompareWithCapture (*track.Root (), capturedRoot);
}

TEST_CASE (ObservationReporting)
{
    Profiler::AllocationGroup testGroup {"ObservationReporting"_us};
    // Test group should have some initial state.
    testGroup.Allocate (150u);
    testGroup.Acquire (80u);

    Recording::Track track;
    auto [capturedRoot, observer] = Profiler::Capture::Start ();

    Recording::RuntimeReporter reporter;
    reporter.Begin (&track, capturedRoot);

    testGroup.Allocate (1000u);
    testGroup.Acquire (350u);
    testGroup.Acquire (600u);
    Profiler::AddMarker ("Hello, world!"_us, testGroup);
    testGroup.Release (1030u);
    testGroup.Free (1100u);

    // Apply all events and check them.

    Recording::GroupUID testGroupUID = Recording::MISSING_GROUP_ID;
    while (const Profiler::Event *sourceEvent = observer.NextEvent ())
    {
        reporter.ReportEvent (*sourceEvent);

        // Ignore events from other groups.
        if (sourceEvent->group == testGroup)
        {
            // Move to the end.
            while (track.MoveToNextEvent ())
            {
            }

            auto checkGroup = [&testGroup, &track, &testGroupUID] (Recording::GroupUID _uid)
            {
                testGroupUID = _uid;
                const Recording::RecordedAllocationGroup *recordedGroup = track.GetGroupByUID (_uid);
                REQUIRE (recordedGroup);
                CHECK (IsSource (testGroup, *recordedGroup));
            };

            const Recording::Event *resultEvent = *track.EventCurrent ();
            switch (resultEvent->type)
            {
            case Recording::EventType::DECLARE_GROUP:
            {
                CHECK_WITH_MESSAGE (false, "We do not create new groups after capture in this test.");
                break;
            }
            case Recording::EventType::ALLOCATE:
            {
                CHECK_EQUAL (sourceEvent->type, Profiler::EventType::ALLOCATE);
                CHECK_EQUAL (sourceEvent->bytes, resultEvent->bytes);
                checkGroup (resultEvent->group);
                break;
            }
            case Recording::EventType::ACQUIRE:
            {
                CHECK_EQUAL (sourceEvent->type, Profiler::EventType::ACQUIRE);
                CHECK_EQUAL (sourceEvent->bytes, resultEvent->bytes);
                checkGroup (resultEvent->group);
                break;
            }
            case Recording::EventType::RELEASE:
            {
                CHECK_EQUAL (sourceEvent->type, Profiler::EventType::RELEASE);
                CHECK_EQUAL (sourceEvent->bytes, resultEvent->bytes);
                checkGroup (resultEvent->group);
                break;
            }
            case Recording::EventType::FREE:
            {
                CHECK_EQUAL (sourceEvent->type, Profiler::EventType::FREE);
                CHECK_EQUAL (sourceEvent->bytes, resultEvent->bytes);
                checkGroup (resultEvent->group);
                break;
            }
            case Recording::EventType::MARKER:
            {
                CHECK_EQUAL (sourceEvent->type, Profiler::EventType::MARKER);
                CHECK_EQUAL (sourceEvent->markerId, resultEvent->markerId);
                checkGroup (resultEvent->scope);
                break;
            }
            }

            CHECK_EQUAL (sourceEvent->timeNs, resultEvent->timeNs);
        }
    }

    CHECK_NOT_EQUAL (testGroupUID, Recording::MISSING_GROUP_ID);

    // Check that result state matches.
    const Recording::RecordedAllocationGroup *recordedGroup = track.GetGroupByUID (testGroupUID);
    REQUIRE (recordedGroup);
    CHECK_EQUAL (recordedGroup->GetReserved (), testGroup.GetReserved ());
    CHECK_EQUAL (recordedGroup->GetAcquired (), testGroup.GetAcquired ());
    CHECK_EQUAL (recordedGroup->GetTotal (), testGroup.GetTotal ());

    // Check undo functionality by reverting to initialized state.
    while (!IsAtInitializationEndMarker (track))
    {
        REQUIRE_WITH_MESSAGE (track.MoveToPreviousEvent (), "Moving to initialization end marker.");
    }

    REQUIRE (track.Root ());
    CompareWithCapture (*track.Root (), capturedRoot);
}

TEST_CASE (UncapturedGroup)
{
    Recording::Track track;
    auto [capturedRoot, observer] = Profiler::Capture::Start ();

    Recording::RuntimeReporter reporter;
    reporter.Begin (&track, capturedRoot);

    while (!IsAtInitializationEndMarker (track))
    {
        REQUIRE_WITH_MESSAGE (track.MoveToNextEvent (), "Moving to initialization end marker.");
    }

    const auto initializationMarkerIterator = track.EventCurrent ();

    Profiler::AllocationGroup testGroup {"UncapturedGroup"_us};
    // Test group should have some initial state.
    constexpr std::size_t ALLOCATION_BYTES = 150u;
    testGroup.Allocate (ALLOCATION_BYTES);
    testGroup.Acquire (80u);

    while (const Profiler::Event *sourceEvent = observer.NextEvent ())
    {
        reporter.ReportEvent (*sourceEvent);
    }

    // Check that uncaptured group is declared after initialization and extract its uid.
    Recording::GroupUID testGroupUID = Recording::MISSING_GROUP_ID;

    for (auto iterator = initializationMarkerIterator; iterator != track.EventEnd (); ++iterator)
    {
        const Recording::Event *event = *iterator;
        if (event->type == Recording::EventType::DECLARE_GROUP)
        {
            CHECK_EQUAL (event->id, testGroup.GetId ());
            testGroupUID = event->uid;
            break;
        }
    }

    CHECK_NOT_EQUAL (testGroupUID, Recording::MISSING_GROUP_ID);

    // Move to the end.
    while (track.MoveToNextEvent ())
    {
    }

    // Check that result state matches.
    const Recording::RecordedAllocationGroup *recordedGroup = track.GetGroupByUID (testGroupUID);
    REQUIRE (recordedGroup);
    CHECK_EQUAL (recordedGroup->GetReserved (), testGroup.GetReserved ());
    CHECK_EQUAL (recordedGroup->GetAcquired (), testGroup.GetAcquired ());
    CHECK_EQUAL (recordedGroup->GetTotal (), testGroup.GetTotal ());
}

TEST_CASE (ReuseRuntimeReporter)
{
    Recording::RuntimeReporter reporter;

    auto doReport = [&reporter] ()
    {
        Recording::Track track;
        Profiler::CapturedAllocationGroup capturedRoot = Profiler::Capture::Start ().first;
        reporter.Begin (&track, capturedRoot);

        // Go to the last event, where all groups are initialized.
        while (!IsAtInitializationEndMarker (track))
        {
            REQUIRE_WITH_MESSAGE (track.MoveToNextEvent (), "Moving to initialization end marker.");
        }

        REQUIRE (track.Root ());
        CompareWithCapture (*track.Root (), capturedRoot);
        reporter.End ();
    };

    doReport ();
    Profiler::AllocationGroup testGroup {"ReuseRuntimeReporter"_us};
    testGroup.Allocate (120u);
    testGroup.Acquire (80u);
    doReport ();
}

TEST_CASE (CorruptedEvents)
{
    Profiler::AllocationGroup testGroup {"CorruptedEvents"_us};
    // Test group should have some initial state.
    constexpr std::size_t ALLOCATION_BYTES = 150u;
    testGroup.Allocate (ALLOCATION_BYTES);

    Profiler::CapturedAllocationGroup capturedRoot = Profiler::Capture::Start ().first;
    auto reportCorruptedEvent = [&capturedRoot] (const Profiler::Event &_event)
    {
        Recording::Track track;
        Recording::RuntimeReporter reporter;
        reporter.Begin (&track, capturedRoot);

        // Go to the last event, where all groups are initialized.
        while (!IsAtInitializationEndMarker (track))
        {
            REQUIRE_WITH_MESSAGE (track.MoveToNextEvent (), "Moving to initialization end marker.");
        }

        reporter.ReportEvent (_event);
        // Next event is corrupted, therefore we should not be able to move.
        CHECK (!track.MoveToNextEvent ());
    };

    reportCorruptedEvent ({
        .type = Profiler::EventType::ACQUIRE,
        .group = testGroup,
        .timeNs = 0u,
        .bytes = ALLOCATION_BYTES,
    });

    reportCorruptedEvent ({
        .type = Profiler::EventType::ACQUIRE,
        .group = testGroup,
        // Capture initialization takes time, therefore we add this 1 second buffer to event time.
        .timeNs = Emergence::Time::NanosecondsSinceStartup () + 1000000000u,
        .bytes = ALLOCATION_BYTES * 2u,
    });

    reportCorruptedEvent ({
        .type = Profiler::EventType::ACQUIRE,
        .group = testGroup,
        // Capture initialization takes time, therefore we add this 1 second buffer to event time.
        .timeNs = Emergence::Time::NanosecondsSinceStartup () + 1000000000u,
        .bytes = ALLOCATION_BYTES * 2u,
    });

    reportCorruptedEvent ({
        .type = Profiler::EventType::RELEASE,
        .group = testGroup,
        // Capture initialization takes time, therefore we add this 1 second buffer to event time.
        .timeNs = Emergence::Time::NanosecondsSinceStartup () + 1000000000u,
        .bytes = ALLOCATION_BYTES,
    });

    reportCorruptedEvent ({
        .type = Profiler::EventType::FREE,
        .group = testGroup,
        // Capture initialization takes time, therefore we add this 1 second buffer to event time.
        .timeNs = Emergence::Time::NanosecondsSinceStartup () + 1000000000u,
        .bytes = ALLOCATION_BYTES * 2u,
    });
}

TEST_CASE (EventIteratorCircling)
{
    Recording::Track track;
    Profiler::CapturedAllocationGroup capturedRoot = Profiler::Capture::Start ().first;

    Recording::RuntimeReporter reporter;
    reporter.Begin (&track, capturedRoot);

    CHECK_NOT_EQUAL (track.EventBegin (), track.EventEnd ());
    CHECK_EQUAL (--track.EventBegin (), track.EventEnd ());
    CHECK_EQUAL (++track.EventEnd (), track.EventBegin ());
}

END_SUITE

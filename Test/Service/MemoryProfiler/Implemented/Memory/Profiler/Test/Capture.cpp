#include <Container/Optional.hpp>
#include <Container/Vector.hpp>

#include <Memory/Profiler/Capture.hpp>
#include <Memory/Profiler/Test/Capture.hpp>

#include <Memory/Profiler/Test/DefaultAllocationGroupStub.hpp>

#include <Testing/Testing.hpp>

namespace Emergence::Memory::Profiler::Test
{
bool CaptureTestIncludeMarker () noexcept
{
    return true;
}

void CheckCaptureConsistency (const CapturedAllocationGroup &_captured, const AllocationGroup &_expectedSource)
{
    CHECK_EQUAL (_captured.GetSource (), _expectedSource);
    CHECK_EQUAL (_captured.GetId (), _expectedSource.GetId ());
    CHECK_EQUAL (_captured.GetReserved (), _expectedSource.GetReserved ());
    CHECK_EQUAL (_captured.GetAcquired (), _expectedSource.GetAcquired ());
    CHECK_EQUAL (_captured.GetTotal (), _expectedSource.GetTotal ());

    auto capturedIterator = _captured.BeginChildren ();
    auto sourceIterator = _expectedSource.BeginChildren ();

    while (capturedIterator != _captured.EndChildren () && sourceIterator != _expectedSource.EndChildren ())
    {
        CheckCaptureConsistency (*capturedIterator, *sourceIterator);
        // Capture must be done in one transaction, therefore capture time must be equal in hierarchy.
        CHECK_EQUAL (_captured.GetCaptureTimeNs (), (*capturedIterator).GetCaptureTimeNs ());

        ++capturedIterator;
        ++sourceIterator;
    }

    CHECK_EQUAL (capturedIterator, _captured.EndChildren ());
    CHECK_EQUAL (sourceIterator, _expectedSource.EndChildren ());
}
} // namespace Emergence::Memory::Profiler::Test

using namespace Emergence::Container;
using namespace Emergence::Memory::Literals;
using namespace Emergence::Memory::Profiler::Test;
using namespace Emergence::Memory::Profiler;

BEGIN_SUITE (Capture)

TEST_CASE (GroupCapture)
{
    static const Emergence::Memory::UniqueString coreId {"GroupCapture::Core"};
    AllocationGroup core {AllocationGroup::Root (), coreId};
    AllocationGroup first {core, "GroupCapture::First"_us};
    AllocationGroup second {core, "GroupCapture::Second"_us};
    AllocationGroup firstChild1 {first, "GroupCapture::FirstChild1"_us};
    AllocationGroup firstChild2 {first, "GroupCapture::FirstChild2"_us};

    firstChild1.Allocate (200u);
    firstChild2.Allocate (300u);
    firstChild2.Acquire (200u);

    second.Allocate (100u);
    second.Acquire (50u);

    CapturedAllocationGroup capturedRoot = Capture::Start ().first;
    for (auto iterator = capturedRoot.BeginChildren (); iterator != capturedRoot.EndChildren (); ++iterator)
    {
        CapturedAllocationGroup group = *iterator;
        if (group.GetId () == coreId)
        {
            CheckCaptureConsistency (group, core);
            return;
        }
    }

    CHECK_WITH_MESSAGE (false, "Captured core group not found!");
}

TEST_CASE (EventCapture)
{
    AllocationGroup group {"EventCapture::Group"_us};
    // We are checking that new observer will not consume "historical" events.
    EventObserver oldOne = Capture::Start ().second;
    group.Allocate (200u);

    EventObserver observer = Capture::Start ().second;
    auto checkEvents = [&observer, &group] (const Vector<Event> &_expectation)
    {
        uint64_t previousEventTime = 0u;
        auto iterator = _expectation.begin ();

        while (const Event *received = observer.NextEvent ())
        {
            if (received->group != group && (iterator == _expectation.end () || iterator->group != received->group))
            {
                // Event belongs to other unexpected group, perhaps to some utility allocators.
                // Unfortunately, we can not silence events from them in this test.
                continue;
            }

            CHECK_NOT_EQUAL (iterator, _expectation.end ());
            if (iterator != _expectation.end ())
            {
                const Event &expected = *iterator;
                ++iterator;
                CHECK_EQUAL (expected.type, received->type);

                if (previousEventTime)
                {
                    CHECK (previousEventTime <= received->timeNs);
                }

                previousEventTime = received->timeNs;
                switch (expected.type)
                {
                case EventType::ALLOCATE:
                case EventType::ACQUIRE:
                case EventType::RELEASE:
                case EventType::FREE:
                {
                    CHECK_EQUAL (expected.bytes, received->bytes);
                    break;
                }
                case EventType::MARKER:
                {
                    CHECK_EQUAL (expected.markerId, received->markerId);
                    break;
                }
                }
            }
        }

        CHECK_EQUAL (iterator, _expectation.end ());
    };

    checkEvents ({});
    group.Acquire (100u);
    group.Free (50u);
    group.Release (25u);
    group.Allocate (1000u);

    checkEvents ({{
                      .type = Emergence::Memory::Profiler::EventType::ACQUIRE,
                      .bytes = 100u,
                  },
                  {
                      .type = Emergence::Memory::Profiler::EventType::FREE,
                      .bytes = 50u,
                  },
                  {
                      .type = Emergence::Memory::Profiler::EventType::RELEASE,
                      .bytes = 25u,
                  },
                  {
                      .type = Emergence::Memory::Profiler::EventType::ALLOCATE,
                      .bytes = 1000u,
                  }});

    static const Emergence::Memory::UniqueString testRootMarker {"TestRootMarker"};
    static const Emergence::Memory::UniqueString testGroupMarker {"TestGroupMarker"};

    AddMarker (testRootMarker);
    AllocationGroup::Root ().Allocate (1000u);
    group.Acquire (350u);
    AddMarker (testGroupMarker, group);

    checkEvents ({{
                      .type = Emergence::Memory::Profiler::EventType::MARKER,
                      .group = AllocationGroup::Root (),
                      .markerId = testRootMarker,
                  },
                  {
                      .type = Emergence::Memory::Profiler::EventType::ALLOCATE,
                      .group = AllocationGroup::Root (),
                      .bytes = 1000u,
                  },
                  {
                      .type = Emergence::Memory::Profiler::EventType::ACQUIRE,
                      .bytes = 350u,
                  },
                  {
                      .type = Emergence::Memory::Profiler::EventType::MARKER,
                      .markerId = testGroupMarker,
                  }});
}

END_SUITE

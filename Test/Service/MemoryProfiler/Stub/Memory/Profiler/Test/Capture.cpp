#include <Memory/Profiler/Capture.hpp>

#include <Testing/Testing.hpp>

using namespace Emergence::Memory::Literals;
using namespace Emergence::Memory::Profiler;

BEGIN_SUITE (Capture)

TEST_CASE (CapturedAllocationGroup)
{
    AllocationGroup group {"CapturedAllocationGroup::First"_us};
    group.Allocate (200u);
    group.Acquire (100u);

    CapturedAllocationGroup captured = Capture::Start ().first;
    CHECK_EQUAL (captured.GetId (), Emergence::Memory::UniqueString {});
    CHECK_EQUAL (captured.GetReserved (), 0u);
    CHECK_EQUAL (captured.GetAcquired (), 0u);
    CHECK_EQUAL (captured.GetTotal (), 0u);

    CHECK_EQUAL (captured.GetSource (), group);
    CHECK_EQUAL (captured.GetSource (), AllocationGroup {});

    auto begin = captured.BeginChildren ();
    auto end = captured.EndChildren ();
    CHECK_EQUAL (begin, end);

    auto beginCopy = begin;
    ++begin;

    CHECK_EQUAL (begin, end);
    CHECK_EQUAL (begin, beginCopy);
    CHECK_EQUAL (begin, beginCopy++);

    beginCopy = end;
    CHECK_EQUAL (beginCopy, end);
    CHECK_EQUAL (begin, beginCopy);
}

TEST_CASE (EventObserver)
{
    AllocationGroup group {"EventObserver::First"_us};
    group.Allocate (200u);
    group.Acquire (100u);

    EventObserver observer = Capture::Start ().second;
    CHECK_EQUAL (observer.NextEvent (), nullptr);

    group.Allocate (200u);
    group.Acquire (100u);
    CHECK_EQUAL (observer.NextEvent (), nullptr);

    AddMarker ("Hello, world!"_us, group);
    CHECK_EQUAL (observer.NextEvent (), nullptr);
}

END_SUITE

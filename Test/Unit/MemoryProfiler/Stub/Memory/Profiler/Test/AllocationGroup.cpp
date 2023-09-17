#include <Memory/Profiler/AllocationGroup.hpp>

#include <Testing/Testing.hpp>

using namespace Emergence::Memory::Literals;
using namespace Emergence::Memory::Profiler;

BEGIN_SUITE (AllocationGroup)

TEST_CASE (Construction)
{
    AllocationGroup first;
    AllocationGroup second {"Construction::Second"_us};
    AllocationGroup third {first, "Construction::Third"_us};
    AllocationGroup secondAgain {"Construction::Second"_us};
}

TEST_CASE (Equality)
{
    AllocationGroup first {"Equality::First"_us};
    AllocationGroup second {"Equality::Second"_us};
    AllocationGroup secondAgain {"Equality::Second"_us};

    CHECK_EQUAL (first, first);
    CHECK_EQUAL (first, second);
    CHECK_EQUAL (first, secondAgain);
    CHECK_EQUAL (second, secondAgain);

    CHECK_EQUAL (first.Hash (), first.Hash ());
    CHECK_EQUAL (first.Hash (), second.Hash ());
    CHECK_EQUAL (first.Hash (), secondAgain.Hash ());
    CHECK_EQUAL (second.Hash (), secondAgain.Hash ());
}

TEST_CASE (Operations)
{
    AllocationGroup group {"Recording::First"_us};
    group.Allocate (100u);
    group.Acquire (100u);
    group.Release (100u);
    group.Free (100u);

    CHECK_EQUAL (group.Parent (), group);
    CHECK_EQUAL (group.GetId (), Emergence::Memory::UniqueString {});
    CHECK_EQUAL (group.GetReserved (), 0u);
    CHECK_EQUAL (group.GetAcquired (), 0u);
    CHECK_EQUAL (group.GetTotal (), 0u);

    auto begin = group.BeginChildren ();
    auto end = group.EndChildren ();
    CHECK_EQUAL (begin, end);

    auto beginCopy = begin;
    ++begin;

    CHECK_EQUAL (begin, end);
    CHECK_EQUAL (begin, beginCopy);
    CHECK_EQUAL (begin, beginCopy++);
    CHECK_EQUAL (*begin, group);

    beginCopy = end;
    CHECK_EQUAL (beginCopy, end);
    CHECK_EQUAL (begin, beginCopy);
}

TEST_CASE (Stack)
{
    AllocationGroup group {"Stack::First"_us};
    CHECK_EQUAL (AllocationGroup::Top (), AllocationGroup::Root ());

    auto placeholder = group.PlaceOnTop ();
    CHECK_EQUAL (AllocationGroup::Top (), group);
}

END_SUITE

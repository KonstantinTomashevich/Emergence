#include <Container/Vector.hpp>

#include <Memory/Profiler/AllocationGroup.hpp>
#include <Memory/Profiler/Test/AllocationGroup.hpp>

#include <Memory/Profiler/Test/DefaultAllocationGroupStub.hpp>

#include <Testing/Testing.hpp>

namespace Emergence::Memory::Profiler::Test
{
bool AllocationGroupTestIncludeMarker () noexcept
{
    return true;
}
} // namespace Emergence::Memory::Profiler::Test

using namespace Emergence::Container;
using namespace Emergence::Memory::Literals;
using namespace Emergence::Memory::Profiler;

BEGIN_SUITE (AllocationGroup)

TEST_CASE (ParentOfRoot)
{
    CHECK_EQUAL (AllocationGroup::Root ().Parent (), AllocationGroup {});
    CHECK_EQUAL (AllocationGroup::Root ().Parent ().Parent (), AllocationGroup {});
}

TEST_CASE (Stack)
{
    AllocationGroup first {"Stack::First"_us};
    AllocationGroup second {"Stack::Second"_us};

    CHECK_EQUAL (first.GetId (), "Stack::First"_us);
    CHECK_EQUAL (first.Parent (), AllocationGroup::Root ());
    CHECK_EQUAL (second.GetId (), "Stack::Second"_us);
    CHECK_EQUAL (second.Parent (), AllocationGroup::Root ());
    CHECK_EQUAL (AllocationGroup::Top (), AllocationGroup::Root ());

    {
        auto firstInStack = first.PlaceOnTop ();
        CHECK_EQUAL (AllocationGroup::Top (), first);

        AllocationGroup firstChild {"Stack::FirstChild"_us};
        CHECK_EQUAL (firstChild.Parent (), first);

        {
            auto secondInStack = second.PlaceOnTop ();
            CHECK_EQUAL (AllocationGroup::Top (), second);

            AllocationGroup secondChild {"Stack::SecondChild"_us};
            CHECK_EQUAL (secondChild.Parent (), second);

            {
                auto firstInStackAgain = first.PlaceOnTop ();
                CHECK_EQUAL (AllocationGroup::Top (), first);

                AllocationGroup firstAnotherChild {"Stack::FirstAnotherChild"_us};
                CHECK_EQUAL (firstAnotherChild.Parent (), first);
            }

            CHECK_EQUAL (AllocationGroup::Top (), second);
        }

        CHECK_EQUAL (AllocationGroup::Top (), first);
    }

    CHECK_EQUAL (AllocationGroup::Top (), AllocationGroup::Root ());
}

TEST_CASE (Recording)
{
    AllocationGroup core {"Recording::Core"_us};
    AllocationGroup first {core, "Recording::First"_us};
    AllocationGroup second {core, "Recording::Second"_us};

    auto check = [] (const AllocationGroup &_group, size_t _reserved, size_t _acquired)
    {
        CHECK_EQUAL (_group.GetReserved (), _reserved);
        CHECK_EQUAL (_group.GetAcquired (), _acquired);
        CHECK_EQUAL (_group.GetTotal (), _reserved + _acquired);
    };

    check (first, 0u, 0u);
    check (second, 0u, 0u);
    check (core, 0u, 0u);

    first.Allocate (100u);
    check (first, 100u, 0u);
    check (second, 0u, 0u);
    check (core, 100u, 0u);

    second.Allocate (200u);
    check (first, 100u, 0u);
    check (second, 200u, 0u);
    check (core, 300u, 0u);

    second.Acquire (100u);
    check (first, 100u, 0u);
    check (second, 100u, 100u);
    check (core, 200u, 100u);

    second.Release (50u);
    check (first, 100u, 0u);
    check (second, 150u, 50u);
    check (core, 250u, 50u);

    first.Free (50u);
    check (first, 50u, 0u);
    check (second, 150u, 50u);
    check (core, 200u, 50u);
}

TEST_CASE (Equality)
{
    AllocationGroup first {"Equality::First"_us};
    AllocationGroup second {"Equality::Second"_us};
    AllocationGroup firstAgain {"Equality::First"_us};

    CHECK_EQUAL (first, first);
    CHECK_EQUAL (first, firstAgain);
    CHECK_EQUAL (second, second);
    CHECK_NOT_EQUAL (first, second);

    CHECK_EQUAL (first.Hash (), first.Hash ());
    CHECK_EQUAL (first.Hash (), firstAgain.Hash ());
    CHECK_EQUAL (second.Hash (), second.Hash ());
    CHECK_NOT_EQUAL (first.Hash (), second.Hash ());

    CHECK_EQUAL (AllocationGroup::Root (), AllocationGroup::Root ());
    CHECK_EQUAL (AllocationGroup::Root ().Hash (), AllocationGroup::Root ().Hash ());

    CHECK_EQUAL (AllocationGroup {}, AllocationGroup {});
    CHECK_EQUAL (AllocationGroup {}.Hash (), AllocationGroup {}.Hash ());
}

TEST_CASE (Iteration)
{
    AllocationGroup core {"Iteration::Core"_us};
    AllocationGroup first {core, "Iteration::First"_us};
    AllocationGroup second {core, "Iteration::Second"_us};
    AllocationGroup firstChild1 {first, "Iteration::FirstChild1"_us};
    AllocationGroup firstChild2 {first, "Iteration::FirstChild2"_us};

    auto check = [] (const AllocationGroup &_group, const Vector<AllocationGroup> &_unorderedExpectation)
    {
        Vector<AllocationGroup> found;
        for (auto iterator = _group.BeginChildren (); iterator != _group.EndChildren (); ++iterator)
        {
            found.emplace_back (*iterator);
        }

        CHECK_EQUAL (found.size (), _unorderedExpectation.size ());
        for (auto iterator = found.begin (); iterator != found.end (); ++iterator)
        {
            // There should be no duplications among groups.
            CHECK (std::find (iterator + 1u, found.end (), *iterator) == found.end ());

            CHECK (std::find (_unorderedExpectation.begin (), _unorderedExpectation.end (), *iterator) !=
                   _unorderedExpectation.end ());
        }
    };

    check (core, {first, second});
    check (first, {firstChild1, firstChild2});
    check (second, {});
}

TEST_CASE (PlaceholderSafety)
{
    AllocationGroup placeholder;
    placeholder.Allocate (100u);
    placeholder.Acquire (100u);
    placeholder.Release (100u);
    placeholder.Free (100u);

    auto begin = placeholder.BeginChildren ();
    auto end = placeholder.EndChildren ();
    CHECK_EQUAL (begin, end);

    CHECK_EQUAL (placeholder.Parent (), placeholder);
    CHECK_EQUAL (placeholder.GetId (), Emergence::Memory::UniqueString {});
    CHECK_EQUAL (placeholder.GetReserved (), 0u);
    CHECK_EQUAL (placeholder.GetAcquired (), 0u);
    CHECK_EQUAL (placeholder.GetTotal (), 0u);

    [[maybe_unused]] uintptr_t hash = placeholder.Hash ();
}

END_SUITE

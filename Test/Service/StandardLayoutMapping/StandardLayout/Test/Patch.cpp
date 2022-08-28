#include <Memory/Profiler/Test/DefaultAllocationGroupStub.hpp>

#include <StandardLayout/MappingRegistration.hpp>
#include <StandardLayout/Patch.hpp>
#include <StandardLayout/PatchBuilder.hpp>
#include <StandardLayout/Test/Patch.hpp>

#include <Testing/Testing.hpp>

namespace Emergence::StandardLayout::Test
{
bool PatchTestIncludeMarker () noexcept
{
    return true;
}

struct Struct
{
    uint32_t uint32 = 0u;
    float floating = 0.0f;

    bool operator== (const Struct &_other) const = default;

    bool operator!= (const Struct &_other) const = default;

    struct Reflection
    {
        FieldId uint32;
        FieldId floating;
        Mapping mapping;
    };

    static const Reflection &Reflect () noexcept;
};

const Struct::Reflection &Struct::Reflect () noexcept
{
    static Reflection reflection = [] ()
    {
        EMERGENCE_MAPPING_REGISTRATION_BEGIN (Struct);
        EMERGENCE_MAPPING_REGISTER_REGULAR (uint32);
        EMERGENCE_MAPPING_REGISTER_REGULAR (floating);
        EMERGENCE_MAPPING_REGISTRATION_END ();
    }();

    return reflection;
}

void DoAdditionTest (const Struct &_initial, const Struct &_first, const Struct &_second)
{
    Patch initialToFirst = PatchBuilder::FromDifference (Struct::Reflect ().mapping, &_first, &_initial);
    Patch firstToSecond = PatchBuilder::FromDifference (Struct::Reflect ().mapping, &_second, &_first);
    Patch added = initialToFirst + firstToSecond;

    Struct target = _initial;
    CHECK_NOT_EQUAL (target, _second);
    added.Apply (&target);
    CHECK_EQUAL (target, _second);
}

void DoSubtractionTest (const Struct &_initial,
                        const Struct &_first,
                        const Struct &_second,
                        const Struct &_expectedSubtracted)
{
    Patch initialToFirst = PatchBuilder::FromDifference (Struct::Reflect ().mapping, &_first, &_initial);
    Patch initialToSecond = PatchBuilder::FromDifference (Struct::Reflect ().mapping, &_second, &_initial);
    Patch subtracted = initialToFirst - initialToSecond;

    Struct target = _initial;
    CHECK_NOT_EQUAL (target, _expectedSubtracted);
    subtracted.Apply (&target);
    CHECK_EQUAL (target, _expectedSubtracted);
}
} // namespace Emergence::StandardLayout::Test

using namespace Emergence::StandardLayout;
using namespace Emergence::StandardLayout::Test;

BEGIN_SUITE (Patch)

TEST_CASE (AdditionNoIntersection)
{
    Struct initial;
    Struct first = initial;
    first.uint32 = 115u;

    Struct second = first;
    second.floating = 178.5647f;
    DoAdditionTest (initial, first, second);
}

TEST_CASE (AdditionIntersection)
{
    Struct initial;
    Struct first = initial;
    first.uint32 = 115u;

    Struct second = first;
    second.uint32 = 12345u;
    DoAdditionTest (initial, first, second);
}

TEST_CASE (SubtractionNoIntersection)
{
    Struct initial;
    Struct first = initial;
    first.uint32 = 115u;

    Struct second = initial;
    first.floating = 12.12f;
    DoSubtractionTest (initial, first, second, first);
}

TEST_CASE (SubtractionIntersection)
{
    Struct initial;
    Struct first = initial;
    first.uint32 = 115u;
    first.floating = 13.15f;

    Struct second = initial;
    second.uint32 = 115u;

    Struct expected = initial;
    expected.floating = first.floating;
    DoSubtractionTest (initial, first, second, expected);
}

TEST_CASE (SubtractionOverride)
{
    Struct initial;
    Struct first = initial;
    first.uint32 = 115u;
    first.floating = 13.15f;

    Struct second = initial;
    second.uint32 = 114u;
    DoSubtractionTest (initial, first, second, first);
}

END_SUITE

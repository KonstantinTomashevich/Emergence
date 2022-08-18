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

namespace
{
struct TestStruct
{
    uint32_t uint32;
    float floating;

    bool operator== (const TestStruct &_other) const = default;

    bool operator!= (const TestStruct &_other) const = default;

    struct Reflection
    {
        FieldId uint32;
        FieldId floating;
        Mapping mapping;
    };

    static const Reflection &Reflect () noexcept;
};

const TestStruct::Reflection &TestStruct::Reflect () noexcept
{
    static Reflection reflection = [] ()
    {
        EMERGENCE_MAPPING_REGISTRATION_BEGIN (TestStruct);
        EMERGENCE_MAPPING_REGISTER_REGULAR (uint32);
        EMERGENCE_MAPPING_REGISTER_REGULAR (floating);
        EMERGENCE_MAPPING_REGISTRATION_END ();
    }();

    return reflection;
}

void DoAdditionTest (const TestStruct &_initial, const TestStruct &_firstVersion, const TestStruct &_secondVersion)
{
    Patch initialToFirst = PatchBuilder::FromDifference (TestStruct::Reflect ().mapping, &_firstVersion, &_initial);
    Patch firstToSecond =
        PatchBuilder::FromDifference (TestStruct::Reflect ().mapping, &_secondVersion, &_firstVersion);
    Patch combined = initialToFirst + firstToSecond;

    TestStruct target = _initial;
    CHECK_NOT_EQUAL (target, _secondVersion);
    combined.Apply (&target);
    CHECK_EQUAL (target, _secondVersion);
}
} // namespace
} // namespace Emergence::StandardLayout::Test

using namespace Emergence::StandardLayout;
using namespace Emergence::StandardLayout::Test;

BEGIN_SUITE (Patch)

TEST_CASE (AdditionNoIntersection)
{
    TestStruct initial;
    TestStruct firstVersion = initial;
    firstVersion.uint32 = 115u;

    TestStruct secondVersion = firstVersion;
    secondVersion.floating = 178.5647f;
    DoAdditionTest (initial, firstVersion, secondVersion);
}

TEST_CASE (AdditionIntersection)
{
    TestStruct initial;
    TestStruct firstVersion = initial;
    firstVersion.uint32 = 115u;

    TestStruct secondVersion = firstVersion;
    secondVersion.uint32 = 12345u;
    DoAdditionTest (initial, firstVersion, secondVersion);
}

END_SUITE

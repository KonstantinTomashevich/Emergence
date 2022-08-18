#include <Container/InplaceVector.hpp>

#include <Memory/Profiler/Test/DefaultAllocationGroupStub.hpp>

#include <StandardLayout/MappingRegistration.hpp>
#include <StandardLayout/PatchBuilder.hpp>
#include <StandardLayout/Test/PatchBuilder.hpp>

#include <Testing/Testing.hpp>

namespace Emergence::StandardLayout::Test
{
bool PatchBuilderTestIncludeMarker () noexcept
{
    return true;
}

struct TestStruct
{
    static constexpr uint8_t ALIVE_FLAG_OFFSET = 0u;
    static constexpr uint8_t POISONED_FLAG_OFFSET = 1u;

    uint8_t flags = 1u << ALIVE_FLAG_OFFSET;

    int8_t int8 = 16;
    int16_t int16 = -6751;
    int32_t int32 = 172649;
    int64_t int64 = -172937461;

    uint8_t uint8 = 16u;
    uint16_t uint16 = 6751u;
    uint32_t uint32 = 172649u;
    uint64_t uint64 = 172937461u;

    float floating = 1627.18274f;
    double doubleFloating = 162.17248172;
    Memory::UniqueString string {"Hello, world!"};

    bool operator== (const TestStruct &_other) const;

    bool operator!= (const TestStruct &_other) const;

    struct Reflection
    {
        FieldId alive;
        FieldId poisoned;

        FieldId int8;
        FieldId int16;
        FieldId int32;
        FieldId int64;

        FieldId uint8;
        FieldId uint16;
        FieldId uint32;
        FieldId uint64;

        FieldId floating;
        FieldId doubleFloating;
        FieldId string;

        Mapping mapping;
    };

    static const Reflection &Reflect () noexcept;
};

bool TestStruct::operator== (const TestStruct &_other) const
{
    // We can not just use default ==, because it will check ::flags as uint variable, while reflection treats it
    // as a bit set and therefore updates only used bits, not whole number.
    constexpr uint8_t USED_BITS = (1u << ALIVE_FLAG_OFFSET) | (1u << POISONED_FLAG_OFFSET);

    return (flags & USED_BITS) == (_other.flags & USED_BITS) && int8 == _other.int8 && int16 == _other.int16 &&
           int32 == _other.int32 && int64 == _other.int64 && uint8 == _other.uint8 && uint16 == _other.uint16 &&
           uint32 == _other.uint32 && uint64 == _other.uint64 && floating == _other.floating &&
           doubleFloating == _other.doubleFloating && string == _other.string;
}

bool TestStruct::operator!= (const TestStruct &_other) const
{
    return !(*this == _other);
}

const TestStruct::Reflection &TestStruct::Reflect () noexcept
{
    static Reflection reflection = [] ()
    {
        EMERGENCE_MAPPING_REGISTRATION_BEGIN (TestStruct);
        EMERGENCE_MAPPING_REGISTER_BIT (alive, flags, ALIVE_FLAG_OFFSET);
        EMERGENCE_MAPPING_REGISTER_BIT (poisoned, flags, POISONED_FLAG_OFFSET);

        EMERGENCE_MAPPING_REGISTER_REGULAR (int8);
        EMERGENCE_MAPPING_REGISTER_REGULAR (int16);
        EMERGENCE_MAPPING_REGISTER_REGULAR (int32);
        EMERGENCE_MAPPING_REGISTER_REGULAR (int64);

        EMERGENCE_MAPPING_REGISTER_REGULAR (uint8);
        EMERGENCE_MAPPING_REGISTER_REGULAR (uint16);
        EMERGENCE_MAPPING_REGISTER_REGULAR (uint32);
        EMERGENCE_MAPPING_REGISTER_REGULAR (uint64);

        EMERGENCE_MAPPING_REGISTER_REGULAR (floating);
        EMERGENCE_MAPPING_REGISTER_REGULAR (doubleFloating);
        EMERGENCE_MAPPING_REGISTER_REGULAR (string);
        EMERGENCE_MAPPING_REGISTRATION_END ();
    }();

    return reflection;
}
} // namespace Emergence::StandardLayout::Test

using namespace Emergence::StandardLayout;
using namespace Emergence::StandardLayout::Test;

BEGIN_SUITE (PatchBuilder)

TEST_CASE (SetFlags)
{
    TestStruct initial;
    TestStruct other = initial;

    PatchBuilder builder;
    builder.Begin (TestStruct::Reflect ().mapping);
    builder.SetBit (TestStruct::Reflect ().alive, false);
    builder.SetBit (TestStruct::Reflect ().poisoned, true);

    Patch patch = builder.End ();
    CHECK_EQUAL (patch.GetTypeMapping (), TestStruct::Reflect ().mapping);

    patch.Apply (&other);
    CHECK_NOT_EQUAL (other, initial);
    CHECK_EQUAL (other.flags, 1u << TestStruct::POISONED_FLAG_OFFSET);
    other.flags = initial.flags;
    CHECK_EQUAL (other, initial);
}

#define SET_REGULAR_TEST(SetterName, field, value)                                                                     \
    TestStruct initial;                                                                                                \
    TestStruct other = initial;                                                                                        \
                                                                                                                       \
    PatchBuilder builder;                                                                                              \
    builder.Begin (TestStruct::Reflect ().mapping);                                                                    \
    builder.Set##SetterName (TestStruct::Reflect ().field, value);                                                     \
    Patch patch = builder.End ();                                                                                      \
                                                                                                                       \
    patch.Apply (&other);                                                                                              \
    CHECK_NOT_EQUAL (other, initial);                                                                                  \
    CHECK_EQUAL (other.field, value);                                                                                  \
    other.field = initial.field;                                                                                       \
    CHECK_EQUAL (other, initial)

TEST_CASE (SetInt8)
{
    SET_REGULAR_TEST (Int8, int8, -1);
}

TEST_CASE (SetInt16)
{
    SET_REGULAR_TEST (Int16, int16, 32);
}

TEST_CASE (SetInt32)
{
    SET_REGULAR_TEST (Int32, int32, -47);
}

TEST_CASE (SetInt64)
{
    SET_REGULAR_TEST (Int64, int32, -17263);
}

TEST_CASE (SetUInt8)
{
    SET_REGULAR_TEST (UInt8, uint8, 11u);
}

TEST_CASE (SetUInt16)
{
    SET_REGULAR_TEST (UInt16, uint16, 17u);
}

TEST_CASE (SetUInt32)
{
    SET_REGULAR_TEST (UInt32, uint32, 51u);
}

TEST_CASE (SetUInt64)
{
    SET_REGULAR_TEST (UInt64, uint32, 172837u);
}

TEST_CASE (SetFloat)
{
    SET_REGULAR_TEST (Float, floating, 1.1023f);
}

TEST_CASE (SetDouble)
{
    SET_REGULAR_TEST (Double, doubleFloating, 1.172638);
}

TEST_CASE (SetUniqueString)
{
    SET_REGULAR_TEST (UniqueString, string, Emergence::Memory::UniqueString {"Welcome, welcome!"});
}

#undef SET_REGULAR_TEST

TEST_CASE (FlagsDifference)
{
    TestStruct initial;
    TestStruct other = initial;
    other.flags = (1u << TestStruct::ALIVE_FLAG_OFFSET) | (1u << TestStruct::POISONED_FLAG_OFFSET);
    Patch patch = PatchBuilder::FromDifference (TestStruct::Reflect ().mapping, &other, &initial);

    CHECK_NOT_EQUAL (other, initial);
    patch.Apply (&initial);
    CHECK_EQUAL (other, initial);
}

#define DIFFERENCE_REGULAR_TEST(field, value)                                                                          \
    TestStruct initial;                                                                                                \
    TestStruct other = initial;                                                                                        \
    other.field = value;                                                                                               \
    Patch patch = PatchBuilder::FromDifference (TestStruct::Reflect ().mapping, &other, &initial);                     \
                                                                                                                       \
    CHECK_NOT_EQUAL (other, initial);                                                                                  \
    patch.Apply (&initial);                                                                                            \
    CHECK_EQUAL (other, initial)

TEST_CASE (Int8Difference)
{
    DIFFERENCE_REGULAR_TEST (int8, -1);
}

TEST_CASE (Int16Difference)
{
    DIFFERENCE_REGULAR_TEST (int16, 29);
}

TEST_CASE (Int32Difference)
{
    DIFFERENCE_REGULAR_TEST (int32, -1726);
}

TEST_CASE (Int64Difference)
{
    DIFFERENCE_REGULAR_TEST (int64, 1727493);
}

TEST_CASE (UInt8Difference)
{
    DIFFERENCE_REGULAR_TEST (uint8, 13u);
}

TEST_CASE (UInt16Difference)
{
    DIFFERENCE_REGULAR_TEST (uint16, 29u);
}

TEST_CASE (UInt32Difference)
{
    DIFFERENCE_REGULAR_TEST (uint32, 1726u);
}

TEST_CASE (UInt64Difference)
{
    DIFFERENCE_REGULAR_TEST (uint64, 1727493u);
}

TEST_CASE (FloatDifference)
{
    DIFFERENCE_REGULAR_TEST (floating, 1.1023f);
}

TEST_CASE (DoubleDifference)
{
    DIFFERENCE_REGULAR_TEST (doubleFloating, 1.1023);
}

TEST_CASE (UniqueStringDifference)
{
    DIFFERENCE_REGULAR_TEST (string, Emergence::Memory::UniqueString {"Welcome, welcome!"});
}

#undef DIFFERENCE_REGULAR_TEST

TEST_CASE (PatchMoveAndCopy)
{
    TestStruct initial;
    TestStruct other = initial;
    TestStruct target = initial;
    other.flags = (1u << TestStruct::ALIVE_FLAG_OFFSET) | (1u << TestStruct::POISONED_FLAG_OFFSET);

    Patch patch = PatchBuilder::FromDifference (TestStruct::Reflect ().mapping, &other, &initial);
    Patch copied = patch;
    Patch moved = std::move (patch);

    CHECK_NOT_EQUAL (target, other);
    copied.Apply (&target);
    CHECK_EQUAL (target, other);

    target = initial;
    CHECK_NOT_EQUAL (target, other);
    moved.Apply (&target);
    CHECK_EQUAL (target, other);
}

TEST_CASE (InplaceVectorDifference)
{
    constexpr std::size_t ITEM_COUNT = 8u;
    Emergence::Container::InplaceVector<TestStruct, ITEM_COUNT> initial;
    initial.EmplaceBack (TestStruct {});
    initial.EmplaceBack (TestStruct {});

    Emergence::Container::InplaceVector<TestStruct, ITEM_COUNT> changed = initial;
    changed[0u].uint32 = 178u;
    changed[1u].floating = 1.024f;
    changed.EmplaceBack (TestStruct {});

    Patch patch = PatchBuilder::FromDifference (decltype (initial)::Reflect ().mapping, &changed, &initial);
    CHECK_NOT_EQUAL (initial, changed);
    patch.Apply (&initial);
    CHECK_EQUAL (initial, changed);
}

TEST_CASE (CombinationNoIntersection)
{
    TestStruct initial;
    TestStruct firstVersion = initial;
    firstVersion.uint32 = 115u;

    TestStruct secondVersion = firstVersion;
    secondVersion.floating = 178.5647f;

    Patch initialToFirst =
        PatchBuilder::FromDifference (decltype (initial)::Reflect ().mapping, &firstVersion, &initial);
    Patch firstToSecond =
        PatchBuilder::FromDifference (decltype (initial)::Reflect ().mapping, &secondVersion, &firstVersion);
    Patch combined = PatchBuilder::Combination (initialToFirst, firstToSecond);

    TestStruct target = initial;
    CHECK_NOT_EQUAL (target, secondVersion);
    combined.Apply (&target);
    CHECK_EQUAL (target, secondVersion);
}

TEST_CASE (CombinationIntersection)
{
    TestStruct initial;
    TestStruct firstVersion = initial;
    firstVersion.uint32 = 115u;

    TestStruct secondVersion = firstVersion;
    secondVersion.uint32 = 12345u;

    Patch initialToFirst =
        PatchBuilder::FromDifference (decltype (initial)::Reflect ().mapping, &firstVersion, &initial);
    Patch firstToSecond =
        PatchBuilder::FromDifference (decltype (initial)::Reflect ().mapping, &secondVersion, &firstVersion);
    Patch combined = PatchBuilder::Combination (initialToFirst, firstToSecond);

    TestStruct target = initial;
    CHECK_NOT_EQUAL (target, secondVersion);
    combined.Apply (&target);
    CHECK_EQUAL (target, secondVersion);
}

END_SUITE

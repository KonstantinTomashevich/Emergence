#include <Container/StringBuilder.hpp>

#include <Memory/Profiler/Test/DefaultAllocationGroupStub.hpp>

#include <Memory/UniqueString.hpp>

#include <StandardLayout/MappingRegistration.hpp>

#include <Testing/Testing.hpp>

namespace Emergence::Container::Test
{
struct IntsSubstructure final
{
    int8_t int8;
    int16_t int16;
    int32_t int32;
    int64_t int64;

    std::uint8_t uint8;
    std::uint16_t uint16;
    std::uint32_t uint32;
    std::uint64_t uint64;

    struct Reflection final
    {
        StandardLayout::FieldId int8;
        StandardLayout::FieldId int16;
        StandardLayout::FieldId int32;
        StandardLayout::FieldId int64;

        StandardLayout::FieldId uint8;
        StandardLayout::FieldId uint16;
        StandardLayout::FieldId uint32;
        StandardLayout::FieldId uint64;

        StandardLayout::Mapping mapping;
    };

    static const Reflection &Reflect () noexcept;
};

const IntsSubstructure::Reflection &IntsSubstructure::Reflect () noexcept
{
    static Reflection reflection = [] ()
    {
        EMERGENCE_MAPPING_REGISTRATION_BEGIN (IntsSubstructure);

        EMERGENCE_MAPPING_REGISTER_REGULAR (int8);
        EMERGENCE_MAPPING_REGISTER_REGULAR (int16);
        EMERGENCE_MAPPING_REGISTER_REGULAR (int32);
        EMERGENCE_MAPPING_REGISTER_REGULAR (int64);

        EMERGENCE_MAPPING_REGISTER_REGULAR (uint8);
        EMERGENCE_MAPPING_REGISTER_REGULAR (uint16);
        EMERGENCE_MAPPING_REGISTER_REGULAR (uint32);
        EMERGENCE_MAPPING_REGISTER_REGULAR (uint64);

        EMERGENCE_MAPPING_REGISTRATION_END ();
    }();

    return reflection;
}

struct FloatsSubstructure final
{
    float floating;
    double doubleFloating;

    struct Reflection final
    {
        StandardLayout::FieldId floating;
        StandardLayout::FieldId doubleFloating;
        StandardLayout::Mapping mapping;
    };

    static const Reflection &Reflect () noexcept;
};

const FloatsSubstructure::Reflection &FloatsSubstructure::Reflect () noexcept
{
    static Reflection reflection = [] ()
    {
        EMERGENCE_MAPPING_REGISTRATION_BEGIN (FloatsSubstructure);
        EMERGENCE_MAPPING_REGISTER_REGULAR (floating);
        EMERGENCE_MAPPING_REGISTER_REGULAR (doubleFloating);
        EMERGENCE_MAPPING_REGISTRATION_END ();
    }();

    return reflection;
}

struct TestStructure final
{
    static constexpr std::size_t ALIVE_OFFSET = 0u;
    static constexpr std::size_t POISONED_OFFSET = 1u;
    static constexpr std::size_t BLESSED_OFFSET = 2u;

    std::uint64_t flags;

    IntsSubstructure ints;
    FloatsSubstructure floats;

    std::uint8_t block[8u];
    char string[24u];
    Memory::UniqueString uniqueString;

    struct Reflection final
    {
        StandardLayout::FieldId alive;
        StandardLayout::FieldId poisoned;
        StandardLayout::FieldId blessed;

        StandardLayout::FieldId ints;
        StandardLayout::FieldId floats;

        StandardLayout::FieldId block;
        StandardLayout::FieldId string;
        StandardLayout::FieldId uniqueString;

        StandardLayout::Mapping mapping;
    };

    static const Reflection &Reflect () noexcept;
};

const TestStructure::Reflection &TestStructure::Reflect () noexcept
{
    static Reflection reflection = [] ()
    {
        EMERGENCE_MAPPING_REGISTRATION_BEGIN (TestStructure);

        EMERGENCE_MAPPING_REGISTER_BIT (alive, flags, ALIVE_OFFSET);
        EMERGENCE_MAPPING_REGISTER_BIT (poisoned, flags, POISONED_OFFSET);
        EMERGENCE_MAPPING_REGISTER_BIT (blessed, flags, BLESSED_OFFSET);

        EMERGENCE_MAPPING_REGISTER_REGULAR (ints);
        EMERGENCE_MAPPING_REGISTER_REGULAR (floats);

        EMERGENCE_MAPPING_REGISTER_BLOCK (block);
        EMERGENCE_MAPPING_REGISTER_STRING (string);
        EMERGENCE_MAPPING_REGISTER_REGULAR (uniqueString);

        EMERGENCE_MAPPING_REGISTRATION_END ();
    }();

    return reflection;
}

static const TestStructure TEST_STRUCTURE {(1u << TestStructure::ALIVE_OFFSET) | (1u << TestStructure::BLESSED_OFFSET),
                                           {
                                               -8,
                                               -1600,
                                               -320000,
                                               -64000000,
                                               8u,
                                               16,
                                               32u,
                                               64u,
                                           },
                                           {
                                               42.123456f,
                                               42.987654,
                                           },
                                           {12u, 55u, 13u, 17u, 11u, 18u, 111u, 243u},
                                           "Hello, inplace string!",
                                           Memory::UniqueString {"Hello, unique string!"}};

struct VectorTestStructure final
{
    Container::Vector<FloatsSubstructure> firstVector;
    Container::Vector<FloatsSubstructure> secondVector;

    struct Reflection final
    {
        StandardLayout::FieldId firstVector;
        StandardLayout::FieldId secondVector;
        StandardLayout::Mapping mapping;
    };

    static const Reflection &Reflect () noexcept;
};

const VectorTestStructure::Reflection &VectorTestStructure::Reflect () noexcept
{
    static Reflection reflection = [] ()
    {
        EMERGENCE_MAPPING_REGISTRATION_BEGIN (VectorTestStructure);
        EMERGENCE_MAPPING_REGISTER_REGULAR (firstVector);
        EMERGENCE_MAPPING_REGISTER_REGULAR (secondVector);
        EMERGENCE_MAPPING_REGISTRATION_END ();
    }();

    return reflection;
}
} // namespace Emergence::Container::Test

using namespace Emergence::Container;
using namespace Emergence::Container::Test;
using namespace Emergence::Container::Literals;
using namespace Emergence::Memory::Literals;

#define BUILD_TEST_STRING(...) String (EMERGENCE_BUILD_STRING (__VA_ARGS__))

BEGIN_SUITE (StringBuilder)

TEST_CASE (TrivialAppend)
{
    CHECK_EQUAL (BUILD_TEST_STRING ("Hello, world!"), "Hello, world!");
    CHECK_EQUAL (BUILD_TEST_STRING ("Knight0"_us), "Knight0");
    CHECK_EQUAL (BUILD_TEST_STRING ("Armor"_s), "Armor");
    CHECK_EQUAL (BUILD_TEST_STRING (std::string_view ("Archer")), "Archer");
    CHECK_EQUAL (BUILD_TEST_STRING (true), "true");
    CHECK_EQUAL (BUILD_TEST_STRING (false), "false");
    CHECK_EQUAL (BUILD_TEST_STRING ('a'), "a");
    CHECK_EQUAL (BUILD_TEST_STRING (-139), "-139");
    CHECK_EQUAL (BUILD_TEST_STRING (-8192789456l), "-8192789456");
    CHECK_EQUAL (BUILD_TEST_STRING (-819278945612ll), "-819278945612");
    CHECK_EQUAL (BUILD_TEST_STRING (8192789456ull), "8192789456");
    CHECK_EQUAL (BUILD_TEST_STRING (819278945612ull), "819278945612");
    CHECK_EQUAL (BUILD_TEST_STRING (11.452f), "11.452000");
    CHECK_EQUAL (BUILD_TEST_STRING (11.45216249563f), "11.452163");
    CHECK_EQUAL (BUILD_TEST_STRING (11.45216249563), "11.452162");
}

TEST_CASE (CombinedAppend)
{
    CHECK_EQUAL (BUILD_TEST_STRING ("Hello, ", "Katrin"_us, ", number ", 42, ", we have a job for you in file "_s, 12,
                                    ".", "34."),
                 "Hello, Katrin, number 42, we have a job for you in file 12.34.");
}

TEST_CASE (FieldReflection)
{
    CHECK_EQUAL (
        BUILD_TEST_STRING (StringBuilder::FieldPointer {
            &TEST_STRUCTURE.flags, TestStructure::Reflect ().mapping.GetField (TestStructure::Reflect ().alive)}),
        "true");

    CHECK_EQUAL (
        BUILD_TEST_STRING (StringBuilder::FieldPointer {
            &TEST_STRUCTURE.flags, TestStructure::Reflect ().mapping.GetField (TestStructure::Reflect ().poisoned)}),
        "false");

    CHECK_EQUAL (
        BUILD_TEST_STRING (StringBuilder::FieldPointer {
            &TEST_STRUCTURE.flags, TestStructure::Reflect ().mapping.GetField (TestStructure::Reflect ().blessed)}),
        "true");

#define BUILD_FIELD_TEST_STRING(Base, Field)                                                                           \
    BUILD_TEST_STRING (StringBuilder::FieldPointer {                                                                   \
        &(Base).Field, decltype (Base)::Reflect ().mapping.GetField (decltype (Base)::Reflect ().Field)})

    CHECK_EQUAL (BUILD_FIELD_TEST_STRING (TEST_STRUCTURE.ints, int8), "-8");
    CHECK_EQUAL (BUILD_FIELD_TEST_STRING (TEST_STRUCTURE.ints, int16), "-1600");
    CHECK_EQUAL (BUILD_FIELD_TEST_STRING (TEST_STRUCTURE.ints, int32), "-320000");
    CHECK_EQUAL (BUILD_FIELD_TEST_STRING (TEST_STRUCTURE.ints, int64), "-64000000");

    CHECK_EQUAL (BUILD_FIELD_TEST_STRING (TEST_STRUCTURE.ints, uint8), "8");
    CHECK_EQUAL (BUILD_FIELD_TEST_STRING (TEST_STRUCTURE.ints, uint16), "16");
    CHECK_EQUAL (BUILD_FIELD_TEST_STRING (TEST_STRUCTURE.ints, uint32), "32");
    CHECK_EQUAL (BUILD_FIELD_TEST_STRING (TEST_STRUCTURE.ints, uint64), "64");

    CHECK_EQUAL (BUILD_FIELD_TEST_STRING (TEST_STRUCTURE.floats, floating), "42.123455");
    CHECK_EQUAL (BUILD_FIELD_TEST_STRING (TEST_STRUCTURE.floats, doubleFloating), "42.987654");

    CHECK_EQUAL (BUILD_FIELD_TEST_STRING (TEST_STRUCTURE, block), "12 55 13 17 11 18 111 243");
    CHECK_EQUAL (BUILD_FIELD_TEST_STRING (TEST_STRUCTURE, string), "Hello, inplace string!");
    CHECK_EQUAL (BUILD_FIELD_TEST_STRING (TEST_STRUCTURE, uniqueString), "Hello, unique string!");

    CHECK_EQUAL (BUILD_FIELD_TEST_STRING (TEST_STRUCTURE, floats),
                 "{ floating = 42.123455, doubleFloating = 42.987654 }");
}

TEST_CASE (MappingReflection)
{
    CHECK_EQUAL (BUILD_TEST_STRING (StringBuilder::ObjectPointer {&TEST_STRUCTURE, TestStructure::Reflect ().mapping}),
                 "{ alive = true, poisoned = false, blessed = true, ints = { int8 = -8, int16 = -1600, int32 = "
                 "-320000, int64 = -64000000, uint8 = 8, uint16 = 16, uint32 = 32, uint64 = 64 }, floats = { floating "
                 "= 42.123455, doubleFloating = 42.987654 }, block = 12 55 13 17 11 18 111 243, string = Hello, "
                 "inplace string!, uniqueString = Hello, unique string! }");
}

TEST_CASE (MappingReflectionWithVector)
{
    VectorTestStructure vectorTestStructure;
    vectorTestStructure.firstVector.emplace_back () = {12.5f, 11.0};
    vectorTestStructure.firstVector.emplace_back () = {12.345f, 7.1};

    vectorTestStructure.secondVector.emplace_back () = {39.5f, 119.0};
    vectorTestStructure.secondVector.emplace_back () = {25.595f, 22.1};
    vectorTestStructure.secondVector.emplace_back () = {3.115f, 1.112};

    CHECK_EQUAL (
        BUILD_TEST_STRING (
            StringBuilder::ObjectPointer {&vectorTestStructure, VectorTestStructure::Reflect ().mapping}),
        "{ firstVector = { { floating = 12.500000, doubleFloating = 11.000000 }, { floating = 12.345000, "
        "doubleFloating = 7.100000 } }, secondVector = { { floating = 39.500000, doubleFloating = 119.000000 }, { "
        "floating = 25.594999, doubleFloating = 22.100000 }, { floating = 3.115000, doubleFloating = 1.112000 } } }");
}

END_SUITE

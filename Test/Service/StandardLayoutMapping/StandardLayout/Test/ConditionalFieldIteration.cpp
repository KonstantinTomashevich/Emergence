#include <Container/InplaceVector.hpp>
#include <Container/Vector.hpp>

#include <Memory/Profiler/Test/DefaultAllocationGroupStub.hpp>

#include <StandardLayout/MappingRegistration.hpp>
#include <StandardLayout/Test/ConditionalFieldIteration.hpp>

#include <Testing/Testing.hpp>

namespace Emergence::StandardLayout::Test
{
bool ConditionalFieldIterationTestIncludeMarker () noexcept
{
    return true;
}

struct InplaceVectorTest final
{
    std::uint64_t id = 0u;
    Container::InplaceVector<std::uint32_t, 4u> vector;

    struct Reflection final
    {
        FieldId id;
        FieldId vector;
        Mapping mapping;
    };

    static const Reflection &Reflect () noexcept;
};

const InplaceVectorTest::Reflection &InplaceVectorTest::Reflect () noexcept
{
    static Reflection reflection = [] ()
    {
        EMERGENCE_MAPPING_REGISTRATION_BEGIN (InplaceVectorTest);
        EMERGENCE_MAPPING_REGISTER_REGULAR (id);
        EMERGENCE_MAPPING_REGISTER_REGULAR (vector);
        EMERGENCE_MAPPING_REGISTRATION_END ();
    }();

    return reflection;
}

struct UnionTest final
{
    UnionTest () noexcept = default;

    std::uint8_t type;

    union
    {
        uint32_t first;

        struct
        {
            uint16_t secondA;
            uint16_t secondB;
        };

        float third;
    };

    struct Reflection final
    {
        FieldId type;
        FieldId first;
        FieldId secondA;
        FieldId secondB;
        FieldId third;
        Mapping mapping;
    };

    static const Reflection &Reflect () noexcept;
};

const UnionTest::Reflection &UnionTest::Reflect () noexcept
{
    static Reflection reflection = [] ()
    {
        EMERGENCE_MAPPING_REGISTRATION_BEGIN (UnionTest);
        EMERGENCE_MAPPING_REGISTER_REGULAR (type);

        EMERGENCE_MAPPING_UNION_VARIANT_BEGIN (type, 0u);
        EMERGENCE_MAPPING_REGISTER_REGULAR (first);
        EMERGENCE_MAPPING_UNION_VARIANT_END ();

        EMERGENCE_MAPPING_UNION_VARIANT_BEGIN (type, 1u);
        EMERGENCE_MAPPING_REGISTER_REGULAR (secondA);
        EMERGENCE_MAPPING_REGISTER_REGULAR (secondB);
        EMERGENCE_MAPPING_UNION_VARIANT_END ();

        EMERGENCE_MAPPING_UNION_VARIANT_BEGIN (type, 2u);
        EMERGENCE_MAPPING_REGISTER_REGULAR (third);
        EMERGENCE_MAPPING_UNION_VARIANT_END ();

        EMERGENCE_MAPPING_REGISTRATION_END ();
    }();

    return reflection;
}

struct CombinedTest final
{
    Container::InplaceVector<UnionTest, 4u> vector;

    struct Reflection final
    {
        FieldId vector;
        Mapping mapping;
    };

    static const Reflection &Reflect () noexcept;
};

const CombinedTest::Reflection &CombinedTest::Reflect () noexcept
{
    static Reflection reflection = [] ()
    {
        EMERGENCE_MAPPING_REGISTRATION_BEGIN (CombinedTest);
        EMERGENCE_MAPPING_REGISTER_REGULAR (vector);
        EMERGENCE_MAPPING_REGISTRATION_END ();
    }();

    return reflection;
}

template <typename Type>
void TestConditionalFieldIteration (const Type &_value, const Container::Vector<FieldId> &_expectedFields)
{
    const Mapping mapping = Type::Reflect ().mapping;
    std::size_t index = 0u;

    for (auto iterator = mapping.BeginConditional (&_value); iterator != mapping.EndConditional (); ++iterator, ++index)
    {
        if (index < _expectedFields.size ())
        {
            CHECK_EQUAL (mapping.GetFieldId (*iterator), _expectedFields[index]);
        }
    }

    CHECK_EQUAL (index, _expectedFields.size ());
}
} // namespace Emergence::StandardLayout::Test

using namespace Emergence::StandardLayout;
using namespace Emergence::StandardLayout::Test;

BEGIN_SUITE (ConditionalFieldIteration)

TEST_CASE (EmptyInplaceVector)
{
#define PROJECT_HELPER(Field)                                                                                          \
    ProjectNestedField (InplaceVectorTest::Reflect ().vector, decltype (InplaceVectorTest::vector)::Reflect ().Field)

    TestConditionalFieldIteration (InplaceVectorTest {}, {
                                                             InplaceVectorTest::Reflect ().id,
                                                             InplaceVectorTest::Reflect ().vector,
                                                             PROJECT_HELPER (count),
                                                         });
}

TEST_CASE (HalfFilledInplaceVector)
{
    InplaceVectorTest test;
    test.vector.EmplaceBack (5u);
    test.vector.EmplaceBack (31u);

    TestConditionalFieldIteration (test, {
                                             InplaceVectorTest::Reflect ().id,
                                             InplaceVectorTest::Reflect ().vector,
                                             PROJECT_HELPER (count),
                                             PROJECT_HELPER (values[0u]),
                                             PROJECT_HELPER (values[1u]),
                                         });
}

TEST_CASE (FullInplaceVector)
{
    InplaceVectorTest test;
    test.vector.EmplaceBack (5u);
    test.vector.EmplaceBack (31u);
    test.vector.EmplaceBack (57u);
    test.vector.EmplaceBack (42u);

    TestConditionalFieldIteration (test, {
                                             InplaceVectorTest::Reflect ().id,
                                             InplaceVectorTest::Reflect ().vector,
                                             PROJECT_HELPER (count),
                                             PROJECT_HELPER (values[0u]),
                                             PROJECT_HELPER (values[1u]),
                                             PROJECT_HELPER (values[2u]),
                                             PROJECT_HELPER (values[3u]),
                                         });

#undef PROJECT_HELPER
}

TEST_CASE (UnionFirstVariant)
{
    UnionTest test;
    test.type = 0u;
    TestConditionalFieldIteration (test, {UnionTest::Reflect ().type, UnionTest::Reflect ().first});
}

TEST_CASE (UnionSecondVariant)
{
    UnionTest test;
    test.type = 1u;
    TestConditionalFieldIteration (
        test, {UnionTest::Reflect ().type, UnionTest::Reflect ().secondA, UnionTest::Reflect ().secondB});
}

TEST_CASE (UnionThirdVariant)
{
    UnionTest test;
    test.type = 2u;
    TestConditionalFieldIteration (test, {UnionTest::Reflect ().type, UnionTest::Reflect ().third});
}

TEST_CASE (Combined)
{
    CombinedTest test;
    test.vector.EmplaceBack ().type = 1u;
    test.vector.EmplaceBack ().type = 0u;
    test.vector.EmplaceBack ().type = 2u;

#define PROJECT_HELPER_2(Field)                                                                                        \
    ProjectNestedField (CombinedTest::Reflect ().vector, decltype (CombinedTest::vector)::Reflect ().Field)

#define PROJECT_HELPER_3(Index, Field)                                                                                 \
    ProjectNestedField (                                                                                               \
        CombinedTest::Reflect ().vector,                                                                               \
        ProjectNestedField (decltype (CombinedTest::vector)::Reflect ().values[Index], UnionTest::Reflect ().Field))

    TestConditionalFieldIteration (test, {
                                             CombinedTest::Reflect ().vector,
                                             PROJECT_HELPER_2 (count),
                                             PROJECT_HELPER_2 (values[0u]),
                                             PROJECT_HELPER_3 (0u, type),
                                             PROJECT_HELPER_3 (0u, secondA),
                                             PROJECT_HELPER_3 (0u, secondB),
                                             PROJECT_HELPER_2 (values[1u]),
                                             PROJECT_HELPER_3 (1u, type),
                                             PROJECT_HELPER_3 (1u, first),
                                             PROJECT_HELPER_2 (values[2u]),
                                             PROJECT_HELPER_3 (2u, type),
                                             PROJECT_HELPER_3 (2u, third),
                                         });

#undef PROJECT_HELPER_3
#undef PROJECT_HELPER_2
}

END_SUITE

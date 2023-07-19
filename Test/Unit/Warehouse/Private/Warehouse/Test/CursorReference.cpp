#include <API/Common/BlockCast.hpp>

#include <Query/Test/Data.hpp>

#include <Reference/Test/Tests.hpp>

#include <Testing/Testing.hpp>

#include <Warehouse/Test/Data.hpp>
#include <Warehouse/Test/Scenario.hpp>

using namespace Emergence::Warehouse::Test;

static Emergence::StandardLayout::FieldId GetPlayedIdField ()
{
    using namespace Emergence::Query::Test;
    using namespace Emergence::StandardLayout;

    return ProjectNestedField (PlayerWithBoundingBox::Reflect ().player, Player::Reflect ().id);
}

static Emergence::Container::Vector<Emergence::Query::Test::Sources::Volumetric::Dimension> GetTestDimensions ()
{
    using namespace Emergence::Query::Test;
    using namespace Emergence::StandardLayout;

    const Emergence::StandardLayout::FieldId boundingBoxField = PlayerWithBoundingBox::Reflect ().boundingBox;

    return {{-100.0f, ProjectNestedField (boundingBoxField, BoundingBox::Reflect ().minX), 100.0f,
             ProjectNestedField (boundingBoxField, BoundingBox::Reflect ().maxX)},
            {-100.0f, ProjectNestedField (boundingBoxField, BoundingBox::Reflect ().minY), 100.0f,
             ProjectNestedField (boundingBoxField, BoundingBox::Reflect ().maxY)}};
}

static Emergence::Container::Vector<Emergence::Query::Test::Storage> GetEnvironment ()
{
    using namespace Emergence::Query::Test;
    return {
        {TestSingleton::Reflect ().mapping, {&TestSingleton::NON_DEFAULT_INSTANCE}, {Sources::Singleton {"Singleton"}}},
        {BoundingBox::Reflect ().mapping, {&BOX_MIN_10_8_4_MAX_11_9_5}, {Sources::UnorderedSequence {"Sequence"}}},
        {PlayerWithBoundingBox::Reflect ().mapping,
         {&HUGO_0_MIN_10_8_4_MAX_11_9_5, &KARL_1_MIN_M2_1_0_MAX_0_4_2},
         {
             Sources::Value {"Value", {GetPlayedIdField ()}},
             Sources::Range {"Range", GetPlayedIdField ()},
             Sources::Signal {"Signal", GetPlayedIdField (), array_cast<std::uint32_t, sizeof (std::uint64_t)> (0u)},
             Sources::Volumetric {"Volumetric", GetTestDimensions ()},
         }}};
}

static void ExecuteFetchSingletonQueryCursorReferenceApiTest (const Emergence::Reference::Test::Scenario &_scenario)
{
    TestReferenceApiDrivers::ForCursor (_scenario, GetEnvironment (), QuerySingletonToRead {{"Singleton", {}}},
                                        &TestSingleton::NON_DEFAULT_INSTANCE);
}

static void ExecuteModifySingletonQueryCursorReferenceApiTest (const Emergence::Reference::Test::Scenario &_scenario)
{
    TestReferenceApiDrivers::ForCursor (_scenario, GetEnvironment (), QuerySingletonToEdit {{"Singleton", {}}},
                                        &TestSingleton::NON_DEFAULT_INSTANCE);
}

static void ExecuteFetchSequenceQueryCursorReferenceApiTest (const Emergence::Reference::Test::Scenario &_scenario)
{
    TestReferenceApiDrivers::ForCursor (_scenario, GetEnvironment (), QueryUnorderedSequenceToRead {{"Sequence", {}}},
                                        &Emergence::Query::Test::BOX_MIN_10_8_4_MAX_11_9_5);
}

static void ExecuteModifySequenceQueryCursorReferenceApiTest (const Emergence::Reference::Test::Scenario &_scenario)
{
    TestReferenceApiDrivers::ForCursor (_scenario, GetEnvironment (), QueryUnorderedSequenceToEdit {{"Sequence", {}}},
                                        &Emergence::Query::Test::BOX_MIN_10_8_4_MAX_11_9_5);
}

static void ExecuteFetchValueQueryCursorReferenceApiTest (const Emergence::Reference::Test::Scenario &_scenario)
{
    using namespace Emergence::Query::Test;
    TestReferenceApiDrivers::ForCursor (_scenario, GetEnvironment (),
                                        QueryValueToRead {{{"Value", {}}, &Queries::ID_0}},
                                        &Emergence::Query::Test::HUGO_0_MIN_10_8_4_MAX_11_9_5);
}

static void ExecuteModifyValueQueryCursorReferenceApiTest (const Emergence::Reference::Test::Scenario &_scenario)
{
    using namespace Emergence::Query::Test;
    TestReferenceApiDrivers::ForCursor (_scenario, GetEnvironment (),
                                        QueryValueToEdit {{{"Value", {}}, &Queries::ID_0}},
                                        &Emergence::Query::Test::HUGO_0_MIN_10_8_4_MAX_11_9_5);
}

static void ExecuteFetchAscendingRangeQueryCursorReferenceApiTest (
    const Emergence::Reference::Test::Scenario &_scenario)
{
    using namespace Emergence::Query::Test;
    TestReferenceApiDrivers::ForCursor (_scenario, GetEnvironment (),
                                        QueryAscendingRangeToRead {{{"Range", {}}, nullptr, nullptr}},
                                        &Emergence::Query::Test::HUGO_0_MIN_10_8_4_MAX_11_9_5);
}

static void ExecuteModifyAscendingRangeQueryCursorReferenceApiTest (
    const Emergence::Reference::Test::Scenario &_scenario)
{
    using namespace Emergence::Query::Test;
    TestReferenceApiDrivers::ForCursor (_scenario, GetEnvironment (),
                                        QueryAscendingRangeToEdit {{{"Range", {}}, nullptr, nullptr}},
                                        &Emergence::Query::Test::HUGO_0_MIN_10_8_4_MAX_11_9_5);
}

static void ExecuteFetchDescendingRangeQueryCursorReferenceApiTest (
    const Emergence::Reference::Test::Scenario &_scenario)
{
    using namespace Emergence::Query::Test;
    TestReferenceApiDrivers::ForCursor (_scenario, GetEnvironment (),
                                        QueryDescendingRangeToRead {{{"Range", {}}, nullptr, nullptr}},
                                        &Emergence::Query::Test::KARL_1_MIN_M2_1_0_MAX_0_4_2);
}

static void ExecuteModifyDescendingRangeQueryCursorReferenceApiTest (
    const Emergence::Reference::Test::Scenario &_scenario)
{
    using namespace Emergence::Query::Test;
    TestReferenceApiDrivers::ForCursor (_scenario, GetEnvironment (),
                                        QueryDescendingRangeToEdit {{{"Range", {}}, nullptr, nullptr}},
                                        &Emergence::Query::Test::KARL_1_MIN_M2_1_0_MAX_0_4_2);
}

static void ExecuteFetchSignalQueryCursorReferenceApiTest (const Emergence::Reference::Test::Scenario &_scenario)
{
    using namespace Emergence::Query::Test;
    TestReferenceApiDrivers::ForCursor (_scenario, GetEnvironment (), QuerySignalToRead {{"Signal", {}}},
                                        &Emergence::Query::Test::HUGO_0_MIN_10_8_4_MAX_11_9_5);
}

static void ExecuteModifySignalQueryCursorReferenceApiTest (const Emergence::Reference::Test::Scenario &_scenario)
{
    using namespace Emergence::Query::Test;
    TestReferenceApiDrivers::ForCursor (_scenario, GetEnvironment (), QuerySignalToEdit {{"Signal", {}}},
                                        &Emergence::Query::Test::HUGO_0_MIN_10_8_4_MAX_11_9_5);
}

static void ExecuteFetchShapeIntersectionQueryCursorReferenceApiTest (
    const Emergence::Reference::Test::Scenario &_scenario)
{
    using namespace Emergence::Query::Test;
    TestReferenceApiDrivers::ForCursor (
        _scenario, GetEnvironment (), QueryShapeIntersectionToRead {{{"Volumetric", {}}, {8.0f, 7.0f}, {10.5f, 9.0f}}},
        &Emergence::Query::Test::HUGO_0_MIN_10_8_4_MAX_11_9_5);
}

static void ExecuteModifyShapeIntersectionQueryCursorReferenceApiTest (
    const Emergence::Reference::Test::Scenario &_scenario)
{
    using namespace Emergence::Query::Test;
    TestReferenceApiDrivers::ForCursor (
        _scenario, GetEnvironment (), QueryShapeIntersectionToEdit {{{"Volumetric", {}}, {8.0f, 7.0f}, {10.5f, 9.0f}}},
        &Emergence::Query::Test::HUGO_0_MIN_10_8_4_MAX_11_9_5);
}

static void ExecuteFetchRayIntersectionQueryCursorReferenceApiTest (
    const Emergence::Reference::Test::Scenario &_scenario)
{
    using namespace Emergence::Query::Test;
    TestReferenceApiDrivers::ForCursor (_scenario, GetEnvironment (),
                                        QueryRayIntersectionToRead {{{"Volumetric", {}}, {7.0f, 9.0f}, {2.0f, 0.0f}}},
                                        &Emergence::Query::Test::HUGO_0_MIN_10_8_4_MAX_11_9_5);
}

static void ExecuteModifyRayIntersectionQueryCursorReferenceApiTest (
    const Emergence::Reference::Test::Scenario &_scenario)
{
    using namespace Emergence::Query::Test;
    TestReferenceApiDrivers::ForCursor (_scenario, GetEnvironment (),
                                        QueryRayIntersectionToEdit {{{"Volumetric", {}}, {7.0f, 9.0f}, {2.0f, 0.0f}}},
                                        &Emergence::Query::Test::HUGO_0_MIN_10_8_4_MAX_11_9_5);
}

// TODO: Insertion query cursors are skipped for now, because they have unique interface.

BEGIN_SUITE (FetchSingletonQueryCursorReferences)

REGISTER_ALL_REFERENCE_TESTS_WITHOUT_ASSIGNMENT (ExecuteFetchSingletonQueryCursorReferenceApiTest)

END_SUITE

BEGIN_SUITE (ModifySingletonQueryCursorReferences)

REGISTER_REFERENCE_TEST (ExecuteModifySingletonQueryCursorReferenceApiTest, ConstructAndDestructSingle)

REGISTER_REFERENCE_TEST (ExecuteModifySingletonQueryCursorReferenceApiTest, MoveChain)

END_SUITE

BEGIN_SUITE (FetchSequenceQueryCursorReferences)

REGISTER_ALL_REFERENCE_TESTS_WITHOUT_ASSIGNMENT (ExecuteFetchSequenceQueryCursorReferenceApiTest)

END_SUITE

BEGIN_SUITE (ModifySequenceQueryCursorReferences)

REGISTER_REFERENCE_TEST (ExecuteModifySequenceQueryCursorReferenceApiTest, ConstructAndDestructSingle)

REGISTER_REFERENCE_TEST (ExecuteModifySequenceQueryCursorReferenceApiTest, MoveChain)

END_SUITE

BEGIN_SUITE (FetchValueQueryCursorReferences)

REGISTER_ALL_REFERENCE_TESTS_WITHOUT_ASSIGNMENT (ExecuteFetchValueQueryCursorReferenceApiTest)

END_SUITE

BEGIN_SUITE (ModifyValueQueryCursorReferences)

REGISTER_REFERENCE_TEST (ExecuteModifyValueQueryCursorReferenceApiTest, ConstructAndDestructSingle)

REGISTER_REFERENCE_TEST (ExecuteModifyValueQueryCursorReferenceApiTest, MoveChain)

END_SUITE

BEGIN_SUITE (FetchAscendingRangeQueryCursorReferences)

REGISTER_ALL_REFERENCE_TESTS_WITHOUT_ASSIGNMENT (ExecuteFetchAscendingRangeQueryCursorReferenceApiTest)

END_SUITE

BEGIN_SUITE (ModifyAscendingRangeQueryCursorReferences)

REGISTER_REFERENCE_TEST (ExecuteModifyAscendingRangeQueryCursorReferenceApiTest, ConstructAndDestructSingle)

REGISTER_REFERENCE_TEST (ExecuteModifyAscendingRangeQueryCursorReferenceApiTest, MoveChain)

END_SUITE

BEGIN_SUITE (FetchDescendingRangeQueryCursorReferences)

REGISTER_ALL_REFERENCE_TESTS_WITHOUT_ASSIGNMENT (ExecuteFetchDescendingRangeQueryCursorReferenceApiTest)

END_SUITE

BEGIN_SUITE (ModifyDescendingRangeQueryCursorReferences)

REGISTER_REFERENCE_TEST (ExecuteModifyDescendingRangeQueryCursorReferenceApiTest, ConstructAndDestructSingle)

REGISTER_REFERENCE_TEST (ExecuteModifyDescendingRangeQueryCursorReferenceApiTest, MoveChain)

END_SUITE

BEGIN_SUITE (FetchSignalQueryCursorReferences)

REGISTER_REFERENCE_TEST (ExecuteFetchSignalQueryCursorReferenceApiTest, ConstructAndDestructSingle)

REGISTER_REFERENCE_TEST (ExecuteFetchSignalQueryCursorReferenceApiTest, MoveChain)

END_SUITE

BEGIN_SUITE (ModifySignalQueryCursorReferences)

REGISTER_REFERENCE_TEST (ExecuteModifySignalQueryCursorReferenceApiTest, ConstructAndDestructSingle)

REGISTER_REFERENCE_TEST (ExecuteModifySignalQueryCursorReferenceApiTest, MoveChain)

END_SUITE

BEGIN_SUITE (FetchShapeIntersectionQueryCursorReferences)

REGISTER_ALL_REFERENCE_TESTS_WITHOUT_ASSIGNMENT (ExecuteFetchShapeIntersectionQueryCursorReferenceApiTest)

END_SUITE

BEGIN_SUITE (ModifyShapeIntersectionQueryCursorReferences)

REGISTER_REFERENCE_TEST (ExecuteModifyShapeIntersectionQueryCursorReferenceApiTest, ConstructAndDestructSingle)

REGISTER_REFERENCE_TEST (ExecuteModifyShapeIntersectionQueryCursorReferenceApiTest, MoveChain)

END_SUITE

BEGIN_SUITE (FetchRayIntersectionQueryCursorReferences)

REGISTER_ALL_REFERENCE_TESTS_WITHOUT_ASSIGNMENT (ExecuteFetchRayIntersectionQueryCursorReferenceApiTest)

END_SUITE

BEGIN_SUITE (ModifyRayIntersectionQueryCursorReferences)

REGISTER_REFERENCE_TEST (ExecuteModifyRayIntersectionQueryCursorReferenceApiTest, ConstructAndDestructSingle)

REGISTER_REFERENCE_TEST (ExecuteModifyRayIntersectionQueryCursorReferenceApiTest, MoveChain)

END_SUITE

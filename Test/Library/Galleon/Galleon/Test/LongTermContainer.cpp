#include <Galleon/Test/Scenario.hpp>

#include <Query/Test/AllParametricQueryTypesInOneStorage.hpp>
#include <Query/Test/Data.hpp>
#include <Query/Test/RangeQueryTests.hpp>
#include <Query/Test/ValueQueryTests.hpp>
#include <Query/Test/VolumetricQueryTests.hpp>

#include <Reference/Test/Tests.hpp>

using namespace Emergence::Galleon::Test;

static Emergence::StandardLayout::FieldId GetPlayedIdField ()
{
    using namespace Emergence::Query::Test;
    using namespace Emergence::StandardLayout;
    return ProjectNestedField (PlayerWithBoundingBox::Reflect ().player, Player::Reflect ().id);
}

static std::vector<Emergence::Query::Test::Sources::Volumetric::Dimension> GetTestDimensions ()
{
    using namespace Emergence::Query::Test;
    using namespace Emergence::StandardLayout;
    const Emergence::StandardLayout::FieldId boundingBoxField = PlayerWithBoundingBox::Reflect ().boundingBox;

    return {{-100.0f, ProjectNestedField (boundingBoxField, BoundingBox::Reflect ().minX), 100.0f,
             ProjectNestedField (boundingBoxField, BoundingBox::Reflect ().maxX)},
            {-100.0f, ProjectNestedField (boundingBoxField, BoundingBox::Reflect ().minY), 100.0f,
             ProjectNestedField (boundingBoxField, BoundingBox::Reflect ().maxY)}};
}

static Emergence::Query::Test::Storage GetTestStorage ()
{
    using namespace Emergence::Query::Test;
    using namespace Emergence::StandardLayout;

    return {PlayerWithBoundingBox::Reflect ().mapping,
            {&HUGO_0_MIN_10_8_4_MAX_11_9_5, &KARL_1_MIN_M2_1_0_MAX_0_4_2},
            {
                Sources::Value {"Value", {GetPlayedIdField ()}},
                Sources::Range {"Range", GetPlayedIdField ()},
                Sources::Volumetric {"Volumetric", GetTestDimensions ()},
            }};
}

static void ExecuteContainerReferenceApiTest (const Emergence::Reference::Test::Scenario &_scenario)
{
    TestReferenceApiDrivers::ForContainerReference (_scenario, GetTestStorage ());
}

static void ExecuteInsertQueryReferenceApiTest (const Emergence::Reference::Test::Scenario &_scenario)
{
    TestReferenceApiDrivers::ForPreparedQuery (_scenario, GetTestStorage (), PrepareLongTermInsertQuery {});
}

static void ExecuteFetchValueQueryReferenceApiTest (const Emergence::Reference::Test::Scenario &_scenario)
{
    TestReferenceApiDrivers::ForPreparedQuery (_scenario, GetTestStorage (),
                                               PrepareLongTermFetchValueQuery {{}, {GetPlayedIdField ()}});
}

static void ExecuteModifyValueQueryReferenceApiTest (const Emergence::Reference::Test::Scenario &_scenario)
{
    TestReferenceApiDrivers::ForPreparedQuery (_scenario, GetTestStorage (),
                                               PrepareLongTermModifyValueQuery {{}, {GetPlayedIdField ()}});
}

static void ExecuteFetchAscendingRangeQueryReferenceApiTest (const Emergence::Reference::Test::Scenario &_scenario)
{
    TestReferenceApiDrivers::ForPreparedQuery (_scenario, GetTestStorage (),
                                               PrepareLongTermFetchAscendingRangeQuery {{}, GetPlayedIdField ()});
}

static void ExecuteModifyAscendingRangeQueryReferenceApiTest (const Emergence::Reference::Test::Scenario &_scenario)
{
    TestReferenceApiDrivers::ForPreparedQuery (_scenario, GetTestStorage (),
                                               PrepareLongTermModifyAscendingRangeQuery {{}, GetPlayedIdField ()});
}

static void ExecuteFetchDescendingRangeQueryReferenceApiTest (const Emergence::Reference::Test::Scenario &_scenario)
{
    TestReferenceApiDrivers::ForPreparedQuery (_scenario, GetTestStorage (),
                                               PrepareLongTermFetchDescendingRangeQuery {{}, GetPlayedIdField ()});
}

static void ExecuteModifyDescendingRangeQueryReferenceApiTest (const Emergence::Reference::Test::Scenario &_scenario)
{
    TestReferenceApiDrivers::ForPreparedQuery (_scenario, GetTestStorage (),
                                               PrepareLongTermModifyDescendingRangeQuery {{}, GetPlayedIdField ()});
}

static void ExecuteFetchShapeIntersectionQueryReferenceApiTest (const Emergence::Reference::Test::Scenario &_scenario)
{
    TestReferenceApiDrivers::ForPreparedQuery (_scenario, GetTestStorage (),
                                               PrepareLongTermFetchShapeIntersectionQuery {{}, GetTestDimensions ()});
}

static void ExecuteModifyShapeIntersectionQueryReferenceApiTest (const Emergence::Reference::Test::Scenario &_scenario)
{
    TestReferenceApiDrivers::ForPreparedQuery (_scenario, GetTestStorage (),
                                               PrepareLongTermModifyShapeIntersectionQuery {{}, GetTestDimensions ()});
}

static void ExecuteFetchRayIntersectionQueryReferenceApiTest (const Emergence::Reference::Test::Scenario &_scenario)
{
    TestReferenceApiDrivers::ForPreparedQuery (_scenario, GetTestStorage (),
                                               PrepareLongTermFetchRayIntersectionQuery {{}, GetTestDimensions ()});
}

static void ExecuteModifyRayIntersectionQueryReferenceApiTest (const Emergence::Reference::Test::Scenario &_scenario)
{
    TestReferenceApiDrivers::ForPreparedQuery (_scenario, GetTestStorage (),
                                               PrepareLongTermModifyRayIntersectionQuery {{}, GetTestDimensions ()});
}

static void ExecuteFetchValueQueryCursorReferenceApiTest (const Emergence::Reference::Test::Scenario &_scenario)
{
    using namespace Emergence::Query::Test;
    TestReferenceApiDrivers::ForCursor (_scenario, GetTestStorage (),
                                        QueryValueToRead {{{"Value", {}}, &Queries::ID_0}},
                                        &Emergence::Query::Test::HUGO_0_MIN_10_8_4_MAX_11_9_5);
}

static void ExecuteModifyValueQueryCursorReferenceApiTest (const Emergence::Reference::Test::Scenario &_scenario)
{
    using namespace Emergence::Query::Test;
    TestReferenceApiDrivers::ForCursor (_scenario, GetTestStorage (),
                                        QueryValueToEdit {{{"Value", {}}, &Queries::ID_0}},
                                        &Emergence::Query::Test::HUGO_0_MIN_10_8_4_MAX_11_9_5);
}

static void ExecuteFetchAscendingRangeQueryCursorReferenceApiTest (
    const Emergence::Reference::Test::Scenario &_scenario)
{
    using namespace Emergence::Query::Test;
    TestReferenceApiDrivers::ForCursor (_scenario, GetTestStorage (),
                                        QueryAscendingRangeToRead {{{"Range", {}}, nullptr, nullptr}},
                                        &Emergence::Query::Test::HUGO_0_MIN_10_8_4_MAX_11_9_5);
}

static void ExecuteModifyAscendingRangeQueryCursorReferenceApiTest (
    const Emergence::Reference::Test::Scenario &_scenario)
{
    using namespace Emergence::Query::Test;
    TestReferenceApiDrivers::ForCursor (_scenario, GetTestStorage (),
                                        QueryAscendingRangeToEdit {{{"Range", {}}, nullptr, nullptr}},
                                        &Emergence::Query::Test::HUGO_0_MIN_10_8_4_MAX_11_9_5);
}

static void ExecuteFetchDescendingRangeQueryCursorReferenceApiTest (
    const Emergence::Reference::Test::Scenario &_scenario)
{
    using namespace Emergence::Query::Test;
    TestReferenceApiDrivers::ForCursor (_scenario, GetTestStorage (),
                                        QueryDescendingRangeToRead {{{"Range", {}}, nullptr, nullptr}},
                                        &Emergence::Query::Test::KARL_1_MIN_M2_1_0_MAX_0_4_2);
}

static void ExecuteModifyDescendingRangeQueryCursorReferenceApiTest (
    const Emergence::Reference::Test::Scenario &_scenario)
{
    using namespace Emergence::Query::Test;
    TestReferenceApiDrivers::ForCursor (_scenario, GetTestStorage (),
                                        QueryDescendingRangeToEdit {{{"Range", {}}, nullptr, nullptr}},
                                        &Emergence::Query::Test::KARL_1_MIN_M2_1_0_MAX_0_4_2);
}

static void ExecuteFetchShapeIntersectionQueryCursorReferenceApiTest (
    const Emergence::Reference::Test::Scenario &_scenario)
{
    using namespace Emergence::Query::Test;
    TestReferenceApiDrivers::ForCursor (
        _scenario, GetTestStorage (), QueryShapeIntersectionToRead {{{"Volumetric", {}}, {8.0f, 7.0f}, {10.5f, 9.0f}}},
        &Emergence::Query::Test::HUGO_0_MIN_10_8_4_MAX_11_9_5);
}

static void ExecuteModifyShapeIntersectionQueryCursorReferenceApiTest (
    const Emergence::Reference::Test::Scenario &_scenario)
{
    using namespace Emergence::Query::Test;
    TestReferenceApiDrivers::ForCursor (
        _scenario, GetTestStorage (), QueryShapeIntersectionToEdit {{{"Volumetric", {}}, {8.0f, 7.0f}, {10.5f, 9.0f}}},
        &Emergence::Query::Test::HUGO_0_MIN_10_8_4_MAX_11_9_5);
}

static void ExecuteFetchRayIntersectionQueryCursorReferenceApiTest (
    const Emergence::Reference::Test::Scenario &_scenario)
{
    using namespace Emergence::Query::Test;
    TestReferenceApiDrivers::ForCursor (_scenario, GetTestStorage (),
                                        QueryRayIntersectionToRead {{{"Volumetric", {}}, {7.0f, 9.0f}, {2.0f, 0.0f}}},
                                        &Emergence::Query::Test::HUGO_0_MIN_10_8_4_MAX_11_9_5);
}

static void ExecuteModifyRayIntersectionQueryCursorReferenceApiTest (
    const Emergence::Reference::Test::Scenario &_scenario)
{
    using namespace Emergence::Query::Test;
    TestReferenceApiDrivers::ForCursor (_scenario, GetTestStorage (),
                                        QueryRayIntersectionToEdit {{{"Volumetric", {}}, {7.0f, 9.0f}, {2.0f, 0.0f}}},
                                        &Emergence::Query::Test::HUGO_0_MIN_10_8_4_MAX_11_9_5);
}

// TODO: InsertQuery::Cursor is skipped for now, because it has unique interface.

BEGIN_SUITE (LongTermContainerReferences)

REGISTER_ALL_REFERENCE_TESTS (ExecuteContainerReferenceApiTest)

END_SUITE

BEGIN_SUITE (LongTermInsertQueryReferences)

REGISTER_ALL_REFERENCE_TESTS_WITHOUT_ASSIGNMENT (ExecuteInsertQueryReferenceApiTest)

END_SUITE

BEGIN_SUITE (LongTermFetchValueQueryReferences)

REGISTER_ALL_REFERENCE_TESTS_WITHOUT_ASSIGNMENT (ExecuteFetchValueQueryReferenceApiTest)

END_SUITE

BEGIN_SUITE (LongTermModifyValueQueryReferences)

REGISTER_ALL_REFERENCE_TESTS_WITHOUT_ASSIGNMENT (ExecuteModifyValueQueryReferenceApiTest)

END_SUITE

BEGIN_SUITE (LongTermFetchAscendingRangeQueryReferences)

REGISTER_ALL_REFERENCE_TESTS_WITHOUT_ASSIGNMENT (ExecuteFetchAscendingRangeQueryReferenceApiTest)

END_SUITE

BEGIN_SUITE (LongTermModifyAscendingRangeQueryReferences)

REGISTER_ALL_REFERENCE_TESTS_WITHOUT_ASSIGNMENT (ExecuteModifyAscendingRangeQueryReferenceApiTest)

END_SUITE

BEGIN_SUITE (LongTermFetchDescendingRangeQueryReferences)

REGISTER_ALL_REFERENCE_TESTS_WITHOUT_ASSIGNMENT (ExecuteFetchDescendingRangeQueryReferenceApiTest)

END_SUITE

BEGIN_SUITE (LongTermModifyDescendingRangeQueryReferences)

REGISTER_ALL_REFERENCE_TESTS_WITHOUT_ASSIGNMENT (ExecuteModifyDescendingRangeQueryReferenceApiTest)

END_SUITE

BEGIN_SUITE (LongTermFetchShapeIntersectionQueryReferences)

REGISTER_ALL_REFERENCE_TESTS_WITHOUT_ASSIGNMENT (ExecuteFetchShapeIntersectionQueryReferenceApiTest)

END_SUITE

BEGIN_SUITE (LongTermModifyShapeIntersectionQueryReferences)

REGISTER_ALL_REFERENCE_TESTS_WITHOUT_ASSIGNMENT (ExecuteModifyShapeIntersectionQueryReferenceApiTest)

END_SUITE

BEGIN_SUITE (LongTermFetchRayIntersectionQueryReferences)

REGISTER_ALL_REFERENCE_TESTS_WITHOUT_ASSIGNMENT (ExecuteFetchRayIntersectionQueryReferenceApiTest)

END_SUITE

BEGIN_SUITE (LongTermModifyRayIntersectionQueryReferences)

REGISTER_ALL_REFERENCE_TESTS_WITHOUT_ASSIGNMENT (ExecuteModifyRayIntersectionQueryReferenceApiTest)

END_SUITE

BEGIN_SUITE (LongTermFetchValueQueryCursorReferences)

REGISTER_ALL_REFERENCE_TESTS_WITHOUT_ASSIGNMENT (ExecuteFetchValueQueryCursorReferenceApiTest)

END_SUITE

BEGIN_SUITE (LongTermModifyValueQueryCursorReferences)

REGISTER_REFERENCE_TEST (ExecuteModifyValueQueryCursorReferenceApiTest, ConstructAndDestructSingle)

REGISTER_REFERENCE_TEST (ExecuteModifyValueQueryCursorReferenceApiTest, MoveChain)

END_SUITE

BEGIN_SUITE (LongTermFetchAscendingRangeQueryCursorReferences)

REGISTER_ALL_REFERENCE_TESTS_WITHOUT_ASSIGNMENT (ExecuteFetchAscendingRangeQueryCursorReferenceApiTest)

END_SUITE

BEGIN_SUITE (LongTermModifyAscendingRangeQueryCursorReferences)

REGISTER_REFERENCE_TEST (ExecuteModifyAscendingRangeQueryCursorReferenceApiTest, ConstructAndDestructSingle)

REGISTER_REFERENCE_TEST (ExecuteModifyAscendingRangeQueryCursorReferenceApiTest, MoveChain)

END_SUITE

BEGIN_SUITE (LongTermFetchDescendingRangeQueryCursorReferences)

REGISTER_ALL_REFERENCE_TESTS_WITHOUT_ASSIGNMENT (ExecuteFetchDescendingRangeQueryCursorReferenceApiTest)

END_SUITE

BEGIN_SUITE (LongTermModifyDescendingRangeQueryCursorReferences)

REGISTER_REFERENCE_TEST (ExecuteModifyDescendingRangeQueryCursorReferenceApiTest, ConstructAndDestructSingle)

REGISTER_REFERENCE_TEST (ExecuteModifyDescendingRangeQueryCursorReferenceApiTest, MoveChain)

END_SUITE

BEGIN_SUITE (LongTermFetchShapeIntersectionQueryCursorReferences)

REGISTER_ALL_REFERENCE_TESTS_WITHOUT_ASSIGNMENT (ExecuteFetchShapeIntersectionQueryCursorReferenceApiTest)

END_SUITE

BEGIN_SUITE (LongTermModifyShapeIntersectionQueryCursorReferences)

REGISTER_REFERENCE_TEST (ExecuteModifyShapeIntersectionQueryCursorReferenceApiTest, ConstructAndDestructSingle)

REGISTER_REFERENCE_TEST (ExecuteModifyShapeIntersectionQueryCursorReferenceApiTest, MoveChain)

END_SUITE

BEGIN_SUITE (LongTermFetchRayIntersectionQueryCursorReferences)

REGISTER_ALL_REFERENCE_TESTS_WITHOUT_ASSIGNMENT (ExecuteFetchRayIntersectionQueryCursorReferenceApiTest)

END_SUITE

BEGIN_SUITE (LongTermModifyRayIntersectionQueryCursorReferences)

REGISTER_REFERENCE_TEST (ExecuteModifyRayIntersectionQueryCursorReferenceApiTest, ConstructAndDestructSingle)

REGISTER_REFERENCE_TEST (ExecuteModifyRayIntersectionQueryCursorReferenceApiTest, MoveChain)

END_SUITE

BEGIN_SUITE (LongTermContainerValueQueries)

REGISTER_ALL_VALUE_QUERY_TESTS (TestQueryApiDriver)

END_SUITE

BEGIN_SUITE (LongTermContainerRangeQueries)

REGISTER_ALL_RANGE_QUERY_TESTS (TestQueryApiDriver)

END_SUITE

BEGIN_SUITE (LongTermContainerVolumetricQueries)

REGISTER_ALL_VOLUMETRIC_QUERY_TESTS (TestQueryApiDriver)

END_SUITE

BEGIN_SUITE (LongTermContainerAllQueriesCombined)

REGISTER_ALL_TESTS_WITH_ALL_PARAMETRIC_QUERY_TYPES_IN_ONE_STORAGE (TestQueryApiDriver)

END_SUITE

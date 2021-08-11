#include <Galleon/Test/Common.hpp>
#include <Galleon/Test/Scenario.hpp>

#include <Query/Test/AllParametricQueryTypesInOneStorage.hpp>
#include <Query/Test/Data.hpp>
#include <Query/Test/RangeQueryTests.hpp>
#include <Query/Test/ValueQueryTests.hpp>
#include <Query/Test/VolumetricQueryTests.hpp>

#include <Reference/Test/Tests.hpp>

#include <Testing/Testing.hpp>

using namespace Emergence::Galleon::Test;

static std::vector <Task> InitTestContainer (const std::string &_name, bool _isAlreadyInitializedTrivialExpectation)
{
    using namespace Emergence::Query::Test;
    if (_isAlreadyInitializedTrivialExpectation)
    {
        return
            {
                AcquireLongTermContainer {
                    {Emergence::Query::Test::PlayerWithBoundingBox::Reflection::GetMapping (), _name}}
            };
    }
    else
    {
        return
            {
                AcquireLongTermContainer {{PlayerWithBoundingBox::Reflection::GetMapping (), _name}},
                PrepareLongTermInsertQuery {{_name, "temporaryInserter"}},
                InsertObjects {"temporaryInserter", {&HUGO_0_MIN_10_8_4_MAX_11_9_5, &KARL_1_MIN_M2_1_0_MAX_0_4_2}},
                Delete <PreparedQueryTag> {"temporaryInserter"},
            };
    }
}

static Emergence::StandardLayout::FieldId GetPlayedIdField ()
{
    using namespace Emergence::Query::Test;
    using namespace Emergence::StandardLayout;
    return ProjectNestedField (PlayerWithBoundingBox::Reflection::player, Player::Reflection::id);
}

static std::vector <Task> CheckIsTestContainerInitialized (bool _shouldBeInitialized)
{
    using namespace Emergence::Query::Test;
    std::vector <Emergence::Galleon::Test::Task> tasks
        {
            AcquireLongTermContainer {{PlayerWithBoundingBox::Reflection::GetMapping (), "temporaryReference"}},
            PrepareLongTermFetchRangeQuery
                {
                    {"temporaryReference", "temporaryQuery"},
                    GetPlayedIdField ()
                },
            QueryRangeToRead {{{"temporaryQuery", "cursor"}, nullptr, nullptr}},
        };

    if (_shouldBeInitialized)
    {
        tasks.emplace_back (
            CursorCheckAllOrdered
                {
                    "cursor",
                    {&HUGO_0_MIN_10_8_4_MAX_11_9_5, &KARL_1_MIN_M2_1_0_MAX_0_4_2}
                });
    }
    else
    {
        tasks.emplace_back (CursorCheckAllOrdered {"cursor", {}});
    }

    tasks +=
        {
            CursorClose {"cursor"},
            Delete <PreparedQueryTag> {"temporaryQuery"},
            Delete <ContainerReferenceTag> {"temporaryReference"},
        };

    return tasks;
}

static void ExecuteContainerReferenceApiTest (const Emergence::Reference::Test::Scenario &_scenario)
{
    ExecuteReferenceApiTest (
        _scenario, InitTestContainer, CheckIsTestContainerInitialized, true);
}

static void ExecuteInsertQueryReferenceApiTest (const Emergence::Reference::Test::Scenario &_scenario)
{
    AdaptPreparedQueryReferenceApiTest (
        _scenario, PrepareLongTermInsertQuery {}, InitTestContainer, CheckIsTestContainerInitialized);
}

static void ExecuteFetchValueQueryReferenceApiTest (const Emergence::Reference::Test::Scenario &_scenario)
{
    AdaptPreparedQueryReferenceApiTest (
        _scenario,
        PrepareLongTermFetchValueQuery {{}, {GetPlayedIdField ()}},
        InitTestContainer, CheckIsTestContainerInitialized);
}

static void ExecuteModifyValueQueryReferenceApiTest (const Emergence::Reference::Test::Scenario &_scenario)
{
    AdaptPreparedQueryReferenceApiTest (
        _scenario,
        PrepareLongTermModifyValueQuery {{}, {GetPlayedIdField ()}},
        InitTestContainer, CheckIsTestContainerInitialized);
}

static void ExecuteFetchRangeQueryReferenceApiTest (const Emergence::Reference::Test::Scenario &_scenario)
{
    AdaptPreparedQueryReferenceApiTest (
        _scenario,
        PrepareLongTermFetchRangeQuery {{}, GetPlayedIdField ()},
        InitTestContainer, CheckIsTestContainerInitialized);
}

static void ExecuteModifyRangeQueryReferenceApiTest (const Emergence::Reference::Test::Scenario &_scenario)
{
    AdaptPreparedQueryReferenceApiTest (
        _scenario,
        PrepareLongTermModifyRangeQuery {{}, GetPlayedIdField ()},
        InitTestContainer, CheckIsTestContainerInitialized);
}

static void ExecuteFetchReversedRangeQueryReferenceApiTest (const Emergence::Reference::Test::Scenario &_scenario)
{
    AdaptPreparedQueryReferenceApiTest (
        _scenario,
        PrepareLongTermFetchReversedRangeQuery {{}, GetPlayedIdField ()},
        InitTestContainer, CheckIsTestContainerInitialized);
}

static void ExecuteModifyReversedRangeQueryReferenceApiTest (const Emergence::Reference::Test::Scenario &_scenario)
{
    AdaptPreparedQueryReferenceApiTest (
        _scenario,
        PrepareLongTermModifyReversedRangeQuery {{}, GetPlayedIdField ()},
        InitTestContainer, CheckIsTestContainerInitialized);
}

static std::vector <Emergence::Query::Test::Sources::Volumetric::Dimension> GetReferenceTestDimensions ()
{
    using namespace Emergence::Query::Test;
    using namespace Emergence::StandardLayout;
    const Emergence::StandardLayout::FieldId boundingBoxField = PlayerWithBoundingBox::Reflection::boundingBox;

    return
        {
            {
                -100.0f,
                ProjectNestedField (boundingBoxField, BoundingBox::Reflection::minX),
                100.0f,
                ProjectNestedField (boundingBoxField, BoundingBox::Reflection::maxX)
            },
            {
                -100.0f,
                ProjectNestedField (boundingBoxField, BoundingBox::Reflection::minY),
                100.0f,
                ProjectNestedField (boundingBoxField, BoundingBox::Reflection::maxY)
            }
        };
}

static void ExecuteFetchShapeIntersectionQueryReferenceApiTest (const Emergence::Reference::Test::Scenario &_scenario)
{
    AdaptPreparedQueryReferenceApiTest (
        _scenario,
        PrepareLongTermFetchShapeIntersectionQuery {{}, GetReferenceTestDimensions ()},
        InitTestContainer, CheckIsTestContainerInitialized);
}

static void ExecuteModifyShapeIntersectionQueryReferenceApiTest (const Emergence::Reference::Test::Scenario &_scenario)
{
    AdaptPreparedQueryReferenceApiTest (
        _scenario,
        PrepareLongTermModifyShapeIntersectionQuery {{}, GetReferenceTestDimensions ()},
        InitTestContainer, CheckIsTestContainerInitialized);
}

static void ExecuteFetchRayIntersectionQueryReferenceApiTest (const Emergence::Reference::Test::Scenario &_scenario)
{
    AdaptPreparedQueryReferenceApiTest (
        _scenario,
        PrepareLongTermFetchRayIntersectionQuery {{}, GetReferenceTestDimensions ()},
        InitTestContainer, CheckIsTestContainerInitialized);
}

static void ExecuteModifyRayIntersectionQueryReferenceApiTest (const Emergence::Reference::Test::Scenario &_scenario)
{
    AdaptPreparedQueryReferenceApiTest (
        _scenario,
        PrepareLongTermModifyRayIntersectionQuery {{}, GetReferenceTestDimensions ()},
        InitTestContainer, CheckIsTestContainerInitialized);
}

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

BEGIN_SUITE (LongTermFetchRangeQueryReferences)

REGISTER_ALL_REFERENCE_TESTS_WITHOUT_ASSIGNMENT (ExecuteFetchRangeQueryReferenceApiTest)

END_SUITE

BEGIN_SUITE (LongTermModifyRangeQueryReferences)

REGISTER_ALL_REFERENCE_TESTS_WITHOUT_ASSIGNMENT (ExecuteModifyRangeQueryReferenceApiTest)

END_SUITE

BEGIN_SUITE (LongTermFetchReversedRangeQueryReferences)

REGISTER_ALL_REFERENCE_TESTS_WITHOUT_ASSIGNMENT (ExecuteFetchReversedRangeQueryReferenceApiTest)

END_SUITE

BEGIN_SUITE (LongTermModifyReversedRangeQueryReferences)

REGISTER_ALL_REFERENCE_TESTS_WITHOUT_ASSIGNMENT (ExecuteModifyReversedRangeQueryReferenceApiTest)

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

BEGIN_SUITE (LongTermContainer::Range)

REGISTER_ALL_RANGE_QUERY_TESTS (TestQueryApiDriver)

END_SUITE

BEGIN_SUITE (LongTermContainer::Value)

REGISTER_ALL_VALUE_QUERY_TESTS (TestQueryApiDriver)

END_SUITE

BEGIN_SUITE (LongTermContainer::Volumetric)

REGISTER_ALL_VOLUMETRIC_QUERY_TESTS (TestQueryApiDriver)

END_SUITE
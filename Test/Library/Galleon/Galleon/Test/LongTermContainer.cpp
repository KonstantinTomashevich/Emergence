#include <Galleon/Test/Common.hpp>
#include <Galleon/Test/Scenario.hpp>

#include <Query/Test/AllParametricQueryTypesInOneStorage.hpp>
#include <Query/Test/Data.hpp>
#include <Query/Test/RangeQueryTests.hpp>
#include <Query/Test/ValueQueryTests.hpp>
#include <Query/Test/VolumetricQueryTests.hpp>

#include <Testing/Testing.hpp>

using namespace Emergence::Galleon::Test;

BEGIN_SUITE (LongTermContainer)

REGISTER_ALL_TESTS_WITH_ALL_PARAMETRIC_QUERY_TYPES_IN_ONE_STORAGE (TestQueryApiDriver)

static std::vector <Task> InitContainerForReferenceTests ()
{
    using namespace Emergence::Query::Test;
    return
        {
            AcquireLongTermContainer {{PlayerWithBoundingBox::Reflection::GetMapping (), "storage"}},
            PrepareLongTermInsertQuery {{"storage", "temporaryInserter"}},
            InsertObjects {"temporaryInserter", {&HUGO_0_MIN_10_8_4_MAX_11_9_5, &KARL_1_MIN_M2_1_0_MAX_0_4_2}},
            Delete <PreparedQueryTag> {"temporaryInserter"},
        };
}

static Emergence::StandardLayout::FieldId GetPlayedIdField ()
{
    using namespace Emergence::Query::Test;
    using namespace Emergence::StandardLayout;
    return ProjectNestedField (PlayerWithBoundingBox::Reflection::player, Player::Reflection::id);
}

static std::vector <Task> CheckReferenceTestsContainerContent (const Task &_checkCursor)
{
    using namespace Emergence::Query::Test;
    return
        std::vector <Emergence::Galleon::Test::Task>
            {
                AcquireLongTermContainer {{PlayerWithBoundingBox::Reflection::GetMapping (), "temporaryReference"}},
                PrepareLongTermFetchRangeQuery
                    {
                        {"temporaryReference", "temporaryQuery"},
                        GetPlayedIdField ()
                    },
                QueryRangeToRead {{{"temporaryQuery", "cursor"}, nullptr, nullptr}},
            } +
        _checkCursor +
        std::vector <Emergence::Galleon::Test::Task>
            {
                CursorClose {"cursor"},
                Delete <PreparedQueryTag> {"temporaryQuery"},
                Delete <ContainerReferenceTag> {"temporaryReference"},
            };
}

static std::vector <Task> CheckThatReferenceTestsContainerInitialized ()
{
    using namespace Emergence::Query::Test;
    return CheckReferenceTestsContainerContent (
        CursorCheckAllOrdered {"cursor", {&HUGO_0_MIN_10_8_4_MAX_11_9_5, &KARL_1_MIN_M2_1_0_MAX_0_4_2}});
}

static std::vector <Task> CheckThatReferenceTestsContainerNotInitialized ()
{
    return CheckReferenceTestsContainerContent (CursorCheck {"cursor", nullptr});
}

static void AdaptQueryReferenceManipulationTest (const Task &_prepareQuery)
{
    Scenario
        {
            TestQueryReferenceManipulation (
                InitContainerForReferenceTests (),
                _prepareQuery,
                CheckThatReferenceTestsContainerInitialized (),
                CheckThatReferenceTestsContainerNotInitialized ())
        };
}

TEST_CASE (ContainerReferenceManipulation)
{
    Scenario
        {
            TestContainerReferenceManipulation (
                InitContainerForReferenceTests (),
                CheckThatReferenceTestsContainerInitialized (),
                CheckThatReferenceTestsContainerNotInitialized ())
        };
}

TEST_CASE (InsertQueryReferenceManipulation)
{
    AdaptQueryReferenceManipulationTest (PrepareLongTermInsertQuery {{"storage", "query"}});
}

TEST_CASE (FetchValueQueryReferenceManipulation)
{
    using namespace Emergence::Query::Test;
    AdaptQueryReferenceManipulationTest (
        PrepareLongTermFetchValueQuery
            {{"storage", "query"},
             {GetPlayedIdField ()}});
}

TEST_CASE (ModifyValueQueryReferenceManipulation)
{
    using namespace Emergence::Query::Test;
    AdaptQueryReferenceManipulationTest (
        PrepareLongTermModifyValueQuery
            {{"storage", "query"},
             {GetPlayedIdField ()}});
}

TEST_CASE (FetchRangeQueryReferenceManipulation)
{
    using namespace Emergence::Query::Test;
    AdaptQueryReferenceManipulationTest (
        PrepareLongTermFetchRangeQuery {{"storage", "query"}, GetPlayedIdField ()});
}

TEST_CASE (ModifyRangeQueryReferenceManipulation)
{
    using namespace Emergence::Query::Test;
    AdaptQueryReferenceManipulationTest (
        PrepareLongTermModifyRangeQuery {{"storage", "query"}, GetPlayedIdField ()});
}

TEST_CASE (FetchReversedRangeQueryReferenceManipulation)
{
    using namespace Emergence::Query::Test;
    AdaptQueryReferenceManipulationTest (
        PrepareLongTermFetchReversedRangeQuery {{"storage", "query"}, GetPlayedIdField ()});
}

TEST_CASE (ModifyReversedRangeQueryReferenceManipulation)
{
    using namespace Emergence::Query::Test;
    AdaptQueryReferenceManipulationTest (
        PrepareLongTermModifyReversedRangeQuery {{"storage", "query"}, GetPlayedIdField ()});
}

std::vector <Emergence::Query::Test::Sources::Volumetric::Dimension> GetReferenceTestDimensions ()
{
    using namespace Emergence::Query::Test;
    using namespace Emergence::StandardLayout;

    return
        {
            {
                -100.0f,
                ProjectNestedField (
                    PlayerWithBoundingBox::Reflection::boundingBox, BoundingBox::Reflection::minX),
                100.0f,
                ProjectNestedField (
                    PlayerWithBoundingBox::Reflection::boundingBox, BoundingBox::Reflection::maxX)
            },
            {
                -100.0f,
                ProjectNestedField (
                    PlayerWithBoundingBox::Reflection::boundingBox, BoundingBox::Reflection::minY),
                100.0f,
                ProjectNestedField (
                    PlayerWithBoundingBox::Reflection::boundingBox, BoundingBox::Reflection::maxY)
            }
        };
}

TEST_CASE (FetchShapeIntersectionQueryReferenceManipulation)
{
    using namespace Emergence::Query::Test;
    AdaptQueryReferenceManipulationTest (
        PrepareLongTermFetchShapeIntersectionQuery {{"storage", "query"}, GetReferenceTestDimensions ()});
}

TEST_CASE (ModifyShapeIntersectionQueryReferenceManipulation)
{
    using namespace Emergence::Query::Test;
    AdaptQueryReferenceManipulationTest (
        PrepareLongTermModifyShapeIntersectionQuery {{"storage", "query"}, GetReferenceTestDimensions ()});
}

TEST_CASE (FetchRayIntersectionQueryReferenceManipulation)
{
    using namespace Emergence::Query::Test;
    AdaptQueryReferenceManipulationTest (
        PrepareLongTermFetchRayIntersectionQuery {{"storage", "query"}, GetReferenceTestDimensions ()});
}

TEST_CASE (ModifyRayIntersectionQueryReferenceManipulation)
{
    using namespace Emergence::Query::Test;
    AdaptQueryReferenceManipulationTest (
        PrepareLongTermModifyRayIntersectionQuery {{"storage", "query"}, GetReferenceTestDimensions ()});
}

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
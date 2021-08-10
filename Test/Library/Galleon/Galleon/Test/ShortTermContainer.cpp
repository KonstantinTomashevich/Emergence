#include <Galleon/Test/Common.hpp>
#include <Galleon/Test/Scenario.hpp>

#include <Query/Test/Data.hpp>
#include <Query/Test/UnorderedSequenceQueryTests.hpp>

#include <Testing/Testing.hpp>

using namespace Emergence::Galleon::Test;

BEGIN_SUITE (ShortTermContainer)

REGISTER_ALL_UNORDERED_SEQUENCE_QUERY_TESTS (TestQueryApiDriver)

static std::vector <Task> InitContainerForReferenceTests ()
{
    using namespace Emergence::Query::Test;
    return
        {
            AcquireShortTermContainer {{Player::Reflection::GetMapping (), "storage"}},
            PrepareShortTermInsertQuery {{"storage", "temporaryInserter"}},
            InsertObjects {"temporaryInserter", {&HUGO_0_ALIVE_STUNNED, &KARL_1_ALIVE_IMMOBILIZED}},
            Delete <PreparedQueryTag> {"temporaryInserter"},
        };
}

static std::vector <Task> CheckReferenceTestsContainerContent (const Task &_checkCursor)
{
    using namespace Emergence::Query::Test;
    return
        std::vector <Emergence::Galleon::Test::Task>
            {
                AcquireShortTermContainer {{Player::Reflection::GetMapping (), "temporaryReference"}},
                PrepareShortTermFetchQuery {{"temporaryReference", "temporaryQuery"}},
                QueryUnorderedSequenceToRead {{"temporaryQuery", "cursor"}},
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
        CursorCheckAllUnordered {"cursor", {&HUGO_0_ALIVE_STUNNED, &KARL_1_ALIVE_IMMOBILIZED}});
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
    AdaptQueryReferenceManipulationTest (PrepareShortTermInsertQuery {{"storage", "query"}});
}

TEST_CASE (FetchQueryReferenceManipulation)
{
    AdaptQueryReferenceManipulationTest (PrepareShortTermFetchQuery {{"storage", "query"}});
}

TEST_CASE (ModifyQueryReferenceManipulation)
{
    AdaptQueryReferenceManipulationTest (PrepareShortTermModifyQuery {{"storage", "query"}});
}

END_SUITE
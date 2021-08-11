#include <Galleon/Test/Common.hpp>
#include <Galleon/Test/Scenario.hpp>

#include <Query/Test/Data.hpp>
#include <Query/Test/UnorderedSequenceQueryTests.hpp>

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
                AcquireShortTermContainer {{Emergence::Query::Test::Player::Reflection::GetMapping (), _name}}
            };
    }
    else
    {
        return
            {
                AcquireShortTermContainer {{Player::Reflection::GetMapping (), _name}},
                PrepareShortTermInsertQuery {{_name, "temporaryInserter"}},
                InsertObjects {"temporaryInserter", {&HUGO_0_ALIVE_STUNNED, &KARL_1_ALIVE_IMMOBILIZED}},
                Delete <PreparedQueryTag> {"temporaryInserter"},
            };
    }
}

static std::vector <Task> CheckIsTestContainerInitialized (bool _shouldBeInitialized)
{
    using namespace Emergence::Query::Test;
    std::vector <Emergence::Galleon::Test::Task> tasks
        {
            AcquireShortTermContainer {{Player::Reflection::GetMapping (), "temporaryReference"}},
            PrepareShortTermFetchQuery {{"temporaryReference", "temporaryQuery"}},
            QueryUnorderedSequenceToRead {{"temporaryQuery", "cursor"}},
        };

    if (_shouldBeInitialized)
    {
        tasks.emplace_back (CursorCheckAllUnordered {"cursor", {&HUGO_0_ALIVE_STUNNED, &KARL_1_ALIVE_IMMOBILIZED}});
    }
    else
    {
        tasks.emplace_back (CursorCheckAllUnordered {"cursor", {}});
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
        _scenario, PrepareShortTermInsertQuery {}, InitTestContainer, CheckIsTestContainerInitialized);
}

static void ExecuteFetchQueryReferenceApiTest (const Emergence::Reference::Test::Scenario &_scenario)
{
    AdaptPreparedQueryReferenceApiTest (
        _scenario, PrepareShortTermFetchQuery {}, InitTestContainer, CheckIsTestContainerInitialized);
}

static void ExecuteModifyQueryReferenceApiTest (const Emergence::Reference::Test::Scenario &_scenario)
{
    AdaptPreparedQueryReferenceApiTest (
        _scenario, PrepareShortTermModifyQuery {}, InitTestContainer, CheckIsTestContainerInitialized);
}

BEGIN_SUITE (ShortTermContainerReferences)

REGISTER_ALL_REFERENCE_TESTS (ExecuteContainerReferenceApiTest)

END_SUITE

BEGIN_SUITE (ShortTermInsertQueryReferences)

REGISTER_ALL_REFERENCE_TESTS_WITHOUT_ASSIGNMENT (ExecuteInsertQueryReferenceApiTest)

END_SUITE

BEGIN_SUITE (ShortTermFetchQueryReferences)

REGISTER_ALL_REFERENCE_TESTS_WITHOUT_ASSIGNMENT (ExecuteFetchQueryReferenceApiTest)

END_SUITE

BEGIN_SUITE (ShortTermModifyQueryReferences)

REGISTER_ALL_REFERENCE_TESTS_WITHOUT_ASSIGNMENT (ExecuteModifyQueryReferenceApiTest)

END_SUITE

BEGIN_SUITE (ShortTermContainerQueries)

REGISTER_ALL_UNORDERED_SEQUENCE_QUERY_TESTS (TestQueryApiDriver)

END_SUITE
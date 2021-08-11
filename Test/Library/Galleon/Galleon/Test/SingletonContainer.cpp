#include <Galleon/Test/Common.hpp>
#include <Galleon/CargoDeck.hpp>

#include <Galleon/Test/Scenario.hpp>

#include <Query/Test/Data.hpp>
#include <Query/Test/SingletonQueryTests.hpp>

#include <Reference/Test/Tests.hpp>

#include <Testing/Testing.hpp>

using namespace Emergence::Galleon::Test;

static std::array <uint8_t, sizeof (Emergence::Query::Test::Player)> UNINITIALIZED_PLAYER {0u};

static std::vector <Task> InitTestContainer (const std::string &_name, bool _isAlreadyInitializedTrivialExpectation)
{
    using namespace Emergence::Query::Test;
    if (_isAlreadyInitializedTrivialExpectation)
    {
        return
            {
                AcquireSingletonContainer {{Emergence::Query::Test::Player::Reflection::GetMapping (), _name}}
            };
    }
    else
    {
        return
            {
                AcquireSingletonContainer {{Player::Reflection::GetMapping (), _name}},
                PrepareSingletonModifyQuery {{_name, "temporaryInserter"}},
                InsertObjects {"temporaryInserter", {&HUGO_0_ALIVE_STUNNED}},
                Delete <PreparedQueryTag> {"temporaryInserter"},
            };
    }
}

static std::vector <Task> CheckIsTestContainerInitialized (bool _shouldBeInitialized)
{
    using namespace Emergence::Query::Test;
    std::vector <Emergence::Galleon::Test::Task> tasks
        {
            AcquireSingletonContainer {{Player::Reflection::GetMapping (), "temporaryReference"}},
            PrepareSingletonFetchQuery {{"temporaryReference", "temporaryQuery"}},
            QuerySingletonToRead {{"temporaryQuery", "cursor"}},
        };

    if (_shouldBeInitialized)
    {
        tasks.emplace_back (CursorCheck {"cursor", &HUGO_0_ALIVE_STUNNED});
    }
    else
    {
        tasks.emplace_back (CursorCheck {"cursor", &UNINITIALIZED_PLAYER});
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

static void ExecuteFetchQueryReferenceApiTest (const Emergence::Reference::Test::Scenario &_scenario)
{
    AdaptPreparedQueryReferenceApiTest (
        _scenario, PrepareSingletonFetchQuery {}, InitTestContainer, CheckIsTestContainerInitialized);
}

static void ExecuteModifyQueryReferenceApiTest (const Emergence::Reference::Test::Scenario &_scenario)
{
    AdaptPreparedQueryReferenceApiTest (
        _scenario, PrepareSingletonModifyQuery {}, InitTestContainer, CheckIsTestContainerInitialized);
}

BEGIN_SUITE (SingletonContainerReferences)

REGISTER_ALL_REFERENCE_TESTS (ExecuteContainerReferenceApiTest)

END_SUITE

BEGIN_SUITE (SingletonFetchQueryReferences)

REGISTER_ALL_REFERENCE_TESTS_WITHOUT_ASSIGNMENT (ExecuteFetchQueryReferenceApiTest)

END_SUITE

BEGIN_SUITE (SingletonModifyQueryReferences)

REGISTER_ALL_REFERENCE_TESTS_WITHOUT_ASSIGNMENT (ExecuteModifyQueryReferenceApiTest)

END_SUITE

BEGIN_SUITE (SingletonContainer)

REGISTER_ALL_SINGLETON_QUERY_TESTS (TestQueryApiDriver)

END_SUITE
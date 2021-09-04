#include <Galleon/Test/Scenario.hpp>

#include <Query/Test/Data.hpp>
#include <Query/Test/SingletonQueryTests.hpp>

#include <Reference/Test/Tests.hpp>

using namespace Emergence::Galleon::Test;

static Emergence::Query::Test::Storage GetTestStorage ()
{
    using namespace Emergence::Query::Test;
    return {Player::Reflect ().mapping, {&HUGO_0_ALIVE_STUNNED}, {Sources::Singleton {"Source"}}};
}

static void ExecuteContainerReferenceApiTest (const Emergence::Reference::Test::Scenario &_scenario)
{
    TestReferenceApiDrivers::ForContainerReference (_scenario, GetTestStorage ());
}

static void ExecuteFetchQueryReferenceApiTest (const Emergence::Reference::Test::Scenario &_scenario)
{
    TestReferenceApiDrivers::ForPreparedQuery (_scenario, GetTestStorage (), PrepareSingletonFetchQuery {});
}

static void ExecuteModifyQueryReferenceApiTest (const Emergence::Reference::Test::Scenario &_scenario)
{
    TestReferenceApiDrivers::ForPreparedQuery (_scenario, GetTestStorage (), PrepareSingletonModifyQuery {});
}

static void ExecuteFetchQueryCursorReferenceApiTest (const Emergence::Reference::Test::Scenario &_scenario)
{
    TestReferenceApiDrivers::ForCursor (_scenario, GetTestStorage (), QuerySingletonToRead {{"Source", {}}},
                                        &Emergence::Query::Test::HUGO_0_ALIVE_STUNNED);
}

static void ExecuteModifyQueryCursorReferenceApiTest (const Emergence::Reference::Test::Scenario &_scenario)
{
    TestReferenceApiDrivers::ForCursor (_scenario, GetTestStorage (), QuerySingletonToEdit {{"Source", {}}},
                                        &Emergence::Query::Test::HUGO_0_ALIVE_STUNNED);
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

BEGIN_SUITE (SingletonFetchQueryCursorReferences)

REGISTER_ALL_REFERENCE_TESTS_WITHOUT_ASSIGNMENT (ExecuteFetchQueryCursorReferenceApiTest)

END_SUITE

BEGIN_SUITE (SingletonModifyQueryCursorReferences)

REGISTER_REFERENCE_TEST (ExecuteModifyQueryCursorReferenceApiTest, ConstructAndDestructSingle)

REGISTER_REFERENCE_TEST (ExecuteModifyQueryCursorReferenceApiTest, MoveChain)

END_SUITE

BEGIN_SUITE (SingletonContainerQueries)

REGISTER_ALL_SINGLETON_QUERY_TESTS (TestQueryApiDriver)

END_SUITE

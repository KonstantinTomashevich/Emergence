#include <Galleon/Test/Scenario.hpp>

#include <Query/Test/Data.hpp>
#include <Query/Test/UnorderedSequenceQueryTests.hpp>

#include <Reference/Test/Tests.hpp>

using namespace Emergence::Galleon::Test;

static Emergence::Query::Test::Storage GetTestStorage ()
{
    using namespace Emergence::Query::Test;
    return {Player::Reflection::GetMapping (), {&HUGO_0_ALIVE_STUNNED}, {Sources::UnorderedSequence {"Source"}}};
}

static void ExecuteContainerReferenceApiTest (const Emergence::Reference::Test::Scenario &_scenario)
{
    TestReferenceApiDrivers::ForContainerReference (_scenario, GetTestStorage ());
}

static void ExecuteInsertQueryReferenceApiTest (const Emergence::Reference::Test::Scenario &_scenario)
{
    TestReferenceApiDrivers::ForPreparedQuery (_scenario, GetTestStorage (), PrepareShortTermInsertQuery {});
}

static void ExecuteFetchQueryReferenceApiTest (const Emergence::Reference::Test::Scenario &_scenario)
{
    TestReferenceApiDrivers::ForPreparedQuery (_scenario, GetTestStorage (), PrepareShortTermFetchQuery {});
}

static void ExecuteModifyQueryReferenceApiTest (const Emergence::Reference::Test::Scenario &_scenario)
{
    TestReferenceApiDrivers::ForPreparedQuery (_scenario, GetTestStorage (), PrepareShortTermModifyQuery {});
}

static void ExecuteFetchQueryCursorReferenceApiTest (const Emergence::Reference::Test::Scenario &_scenario)
{
    TestReferenceApiDrivers::ForCursor (_scenario, GetTestStorage (), QueryUnorderedSequenceToRead {{"Source", {}}},
                                        &Emergence::Query::Test::HUGO_0_ALIVE_STUNNED);
}

static void ExecuteModifyQueryCursorReferenceApiTest (const Emergence::Reference::Test::Scenario &_scenario)
{
    TestReferenceApiDrivers::ForCursor (_scenario, GetTestStorage (), QueryUnorderedSequenceToEdit {{"Source", {}}},
                                        &Emergence::Query::Test::HUGO_0_ALIVE_STUNNED);
}

// TODO: InsertQuery::Cursor is skipped for now, because it has unique interface.

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

BEGIN_SUITE (ShortTermFetchQueryCursorReferences)

REGISTER_ALL_REFERENCE_TESTS_WITHOUT_ASSIGNMENT (ExecuteFetchQueryCursorReferenceApiTest)

END_SUITE

BEGIN_SUITE (ShortTermModifyQueryCursorReferences)

REGISTER_REFERENCE_TEST (ExecuteModifyQueryCursorReferenceApiTest, ConstructAndDestructSingle)

REGISTER_REFERENCE_TEST (ExecuteModifyQueryCursorReferenceApiTest, MoveChain)

END_SUITE

BEGIN_SUITE (ShortTermContainerQueries)

REGISTER_ALL_UNORDERED_SEQUENCE_QUERY_TESTS (TestQueryApiDriver)

END_SUITE
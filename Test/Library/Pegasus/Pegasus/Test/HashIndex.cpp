#include <Pegasus/Test/Scenario.hpp>

#include <Query/Test/Data.hpp>
#include <Query/Test/DataTypes.hpp>
#include <Query/Test/ValueQueryTests.hpp>

#include <Reference/Test/Tests.hpp>

using namespace Emergence::Pegasus::Test;

static Emergence::Query::Test::Storage GetTestStorage ()
{
    using namespace Emergence::Query::Test;
    return
    {
        Player::Reflection::GetMapping (),
        {&HUGO_0_ALIVE_STUNNED},
        {Sources::Value {"Source", {Player::Reflection::id}}}
    };
}

static void ExecuteIndexReferenceApiTest (const Emergence::Reference::Test::Scenario &_scenario)
{
    ReferenceApiTestImporters::ForIndexReference (_scenario, GetTestStorage ());
}

static void ExecuteReadCursorReferenceApiTest (const Emergence::Reference::Test::Scenario &_scenario)
{
    ReferenceApiTestImporters::ForCursor (
        _scenario, GetTestStorage (), QueryValueToRead {{{}, &Emergence::Query::Test::Queries::ID_0}},
        &Emergence::Query::Test::HUGO_0_ALIVE_STUNNED);
}

static void ExecuteEditCursorReferenceApiTest (const Emergence::Reference::Test::Scenario &_scenario)
{
    ReferenceApiTestImporters::ForCursor (
        _scenario, GetTestStorage (), QueryValueToEdit {{{}, &Emergence::Query::Test::Queries::ID_0}},
        &Emergence::Query::Test::HUGO_0_ALIVE_STUNNED);
}

BEGIN_SUITE (HashIndexReference)

REGISTER_ALL_REFERENCE_TESTS (ExecuteIndexReferenceApiTest)

END_SUITE

BEGIN_SUITE (HashIndexReadCursorReference)

REGISTER_ALL_REFERENCE_TESTS_WITHOUT_ASSIGNMENT (ExecuteReadCursorReferenceApiTest)

END_SUITE

BEGIN_SUITE (HashIndexEditCursorReference)

REGISTER_REFERENCE_TEST (ExecuteEditCursorReferenceApiTest, ConstructAndDestructSingle)

REGISTER_REFERENCE_TEST (ExecuteEditCursorReferenceApiTest, MoveChain)

END_SUITE

BEGIN_SUITE (HashIndexQueries)

REGISTER_ALL_VALUE_QUERY_TESTS (TestQueryApiDrivers::CreateIndicesThanInsertRecords)

TEST_CASE (InsertBeforeCreation)
{
    TestQueryApiDrivers::InsertRecordsThanCreateIndices (Emergence::Query::Test::ValueQuery::SimpleLookup ());
}

END_SUITE
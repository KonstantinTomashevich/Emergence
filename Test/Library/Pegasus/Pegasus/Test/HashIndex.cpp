#include <Pegasus/Test/Scenario.hpp>

#include <Query/Test/Data.hpp>
#include <Query/Test/DataTypes.hpp>
#include <Query/Test/ValueQueryTests.hpp>

#include <Reference/Test/Tests.hpp>

using namespace Emergence::Pegasus::Test;

static const char *const TEST_REPRESENTATION_NAME = "source";

static void ExecuteReferenceApiTest (const std::vector <Task> &_importedScenario)
{
    std::vector <Task> tasks
        {
            CreateHashIndex {TEST_REPRESENTATION_NAME, {Emergence::Query::Test::Player::Reflection::id}},
            OpenAllocator {},
            AllocateAndInit {&Emergence::Query::Test::HUGO_0_ALIVE_STUNNED},
            CloseAllocator {},
        };

    tasks += _importedScenario;
    tasks.emplace_back (DropIndex {TEST_REPRESENTATION_NAME});
    Scenario (Emergence::Query::Test::Player::Reflection::GetMapping (), tasks);
}

static void ExecuteIndexReferenceApiTest (const Emergence::Reference::Test::Scenario &_scenario)
{
    ExecuteReferenceApiTest (
        ReferenceApiTestImporters::ForIndexReference (_scenario, TEST_REPRESENTATION_NAME));
}

static void ExecuteReadCursorReferenceApiTest (const Emergence::Reference::Test::Scenario &_scenario)
{
    ExecuteReferenceApiTest (ReferenceApiTestImporters::ForCursor (
        _scenario, TEST_REPRESENTATION_NAME, QueryValueToRead {{{}, &Emergence::Query::Test::Queries::ID_0}},
        &Emergence::Query::Test::HUGO_0_ALIVE_STUNNED));
}

static void ExecuteEditCursorReferenceApiTest (const Emergence::Reference::Test::Scenario &_scenario)
{
    ExecuteReferenceApiTest (ReferenceApiTestImporters::ForCursor (
        _scenario, TEST_REPRESENTATION_NAME, QueryValueToEdit {{{}, &Emergence::Query::Test::Queries::ID_0}},
        &Emergence::Query::Test::HUGO_0_ALIVE_STUNNED));
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
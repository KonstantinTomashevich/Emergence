#include <Pegasus/Test/Scenario.hpp>

#include <Query/Test/Data.hpp>
#include <Query/Test/DataTypes.hpp>
#include <Query/Test/RangeQueryTests.hpp>

#include <Reference/Test/Tests.hpp>

#include <StandardLayout/MappingBuilder.hpp>

using namespace Emergence::Pegasus::Test;

static const char *const TEST_REPRESENTATION_NAME = "source";

static void ExecuteReferenceApiTest (const std::vector <Task> &_importedScenario)
{
    std::vector <Task> tasks
        {
            CreateOrderedIndex {TEST_REPRESENTATION_NAME, Emergence::Query::Test::Player::Reflection::id},
            OpenAllocator {},
            AllocateAndInit {&Emergence::Query::Test::HUGO_0_ALIVE_STUNNED},
            AllocateAndInit {&Emergence::Query::Test::KARL_1_ALIVE_IMMOBILIZED},
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
        _scenario, TEST_REPRESENTATION_NAME, QueryRangeToRead {{{}, nullptr, nullptr}},
        &Emergence::Query::Test::HUGO_0_ALIVE_STUNNED));
}

static void ExecuteEditCursorReferenceApiTest (const Emergence::Reference::Test::Scenario &_scenario)
{
    ExecuteReferenceApiTest (ReferenceApiTestImporters::ForCursor (
        _scenario, TEST_REPRESENTATION_NAME, QueryRangeToEdit {{{}, nullptr, nullptr}},
        &Emergence::Query::Test::HUGO_0_ALIVE_STUNNED));
}

static void ExecuteReversedReadCursorReferenceApiTest (const Emergence::Reference::Test::Scenario &_scenario)
{
    ExecuteReferenceApiTest (ReferenceApiTestImporters::ForCursor (
        _scenario, TEST_REPRESENTATION_NAME, QueryReversedRangeToRead {{{}, nullptr, nullptr}},
        &Emergence::Query::Test::KARL_1_ALIVE_IMMOBILIZED));
}

static void ExecuteReversedEditCursorReferenceApiTest (const Emergence::Reference::Test::Scenario &_scenario)
{
    ExecuteReferenceApiTest (ReferenceApiTestImporters::ForCursor (
        _scenario, TEST_REPRESENTATION_NAME, QueryReversedRangeToEdit {{{}, nullptr, nullptr}},
        &Emergence::Query::Test::KARL_1_ALIVE_IMMOBILIZED));
}

BEGIN_SUITE (OrderedIndexReference)

REGISTER_ALL_REFERENCE_TESTS (ExecuteIndexReferenceApiTest)

END_SUITE

BEGIN_SUITE (OrderedIndexReadCursorReference)

REGISTER_ALL_REFERENCE_TESTS_WITHOUT_ASSIGNMENT (ExecuteReadCursorReferenceApiTest)

END_SUITE

BEGIN_SUITE (OrderedIndexEditCursorReference)

REGISTER_REFERENCE_TEST (ExecuteEditCursorReferenceApiTest, ConstructAndDestructSingle)

REGISTER_REFERENCE_TEST (ExecuteEditCursorReferenceApiTest, MoveChain)

END_SUITE

BEGIN_SUITE (OrderedIndexReversedReadCursorReference)

REGISTER_ALL_REFERENCE_TESTS_WITHOUT_ASSIGNMENT (ExecuteReversedReadCursorReferenceApiTest)

END_SUITE

BEGIN_SUITE (OrderedIndexReversedEditCursorReference)

REGISTER_REFERENCE_TEST (ExecuteReversedEditCursorReferenceApiTest, ConstructAndDestructSingle)

REGISTER_REFERENCE_TEST (ExecuteReversedEditCursorReferenceApiTest, MoveChain)

END_SUITE

BEGIN_SUITE (OrderedIndexQueries)

REGISTER_ALL_RANGE_QUERY_TESTS (TestQueryApiDrivers::CreateIndicesThanInsertRecords)

TEST_CASE (InsertBeforeCreation)
{
    TestQueryApiDrivers::InsertRecordsThanCreateIndices (Emergence::Query::Test::RangeQuery::SimpleLookups ());
}

END_SUITE
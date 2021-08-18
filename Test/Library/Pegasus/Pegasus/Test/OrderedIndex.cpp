#include <Pegasus/Test/Scenario.hpp>

#include <Query/Test/Data.hpp>
#include <Query/Test/DataTypes.hpp>
#include <Query/Test/RangeQueryTests.hpp>

#include <Reference/Test/Tests.hpp>

#include <StandardLayout/MappingBuilder.hpp>

using namespace Emergence::Pegasus::Test;

static Emergence::Query::Test::Storage GetTestStorage ()
{
    using namespace Emergence::Query::Test;
    return
        {
            Player::Reflection::GetMapping (),
            {&HUGO_0_ALIVE_STUNNED, &KARL_1_ALIVE_IMMOBILIZED},
            {Sources::Range {"Source", Player::Reflection::id}}
        };
}

static void ExecuteIndexReferenceApiTest (const Emergence::Reference::Test::Scenario &_scenario)
{
    ReferenceApiTestImporters::ForIndexReference (_scenario, GetTestStorage ());
}

static void ExecuteAscendingReadCursorReferenceApiTest (const Emergence::Reference::Test::Scenario &_scenario)
{
    ReferenceApiTestImporters::ForCursor (
        _scenario, GetTestStorage (), QueryAscendingRangeToRead {{{}, nullptr, nullptr}},
        &Emergence::Query::Test::HUGO_0_ALIVE_STUNNED);
}

static void ExecuteAscendingEditCursorReferenceApiTest (const Emergence::Reference::Test::Scenario &_scenario)
{
    ReferenceApiTestImporters::ForCursor (
        _scenario, GetTestStorage (), QueryAscendingRangeToEdit {{{}, nullptr, nullptr}},
        &Emergence::Query::Test::HUGO_0_ALIVE_STUNNED);
}

static void ExecuteDescendingReadCursorReferenceApiTest (const Emergence::Reference::Test::Scenario &_scenario)
{
    ReferenceApiTestImporters::ForCursor (
        _scenario, GetTestStorage (), QueryDescendingRangeToRead {{{}, nullptr, nullptr}},
        &Emergence::Query::Test::KARL_1_ALIVE_IMMOBILIZED);
}

static void ExecuteDescendingEditCursorReferenceApiTest (const Emergence::Reference::Test::Scenario &_scenario)
{
    ReferenceApiTestImporters::ForCursor (
        _scenario, GetTestStorage (), QueryDescendingRangeToEdit {{{}, nullptr, nullptr}},
        &Emergence::Query::Test::KARL_1_ALIVE_IMMOBILIZED);
}

BEGIN_SUITE (OrderedIndexReference)

REGISTER_ALL_REFERENCE_TESTS (ExecuteIndexReferenceApiTest)

END_SUITE

BEGIN_SUITE (OrderedIndexAscendingReadCursorReference)

REGISTER_ALL_REFERENCE_TESTS_WITHOUT_ASSIGNMENT (ExecuteAscendingReadCursorReferenceApiTest)

END_SUITE

BEGIN_SUITE (OrderedIndexAscendingEditCursorReference)

REGISTER_REFERENCE_TEST (ExecuteAscendingEditCursorReferenceApiTest, ConstructAndDestructSingle)

REGISTER_REFERENCE_TEST (ExecuteAscendingEditCursorReferenceApiTest, MoveChain)

END_SUITE

BEGIN_SUITE (OrderedIndexDescendingReadCursorReference)

REGISTER_ALL_REFERENCE_TESTS_WITHOUT_ASSIGNMENT (ExecuteDescendingReadCursorReferenceApiTest)

END_SUITE

BEGIN_SUITE (OrderedIndexDescendingEditCursorReference)

REGISTER_REFERENCE_TEST (ExecuteDescendingEditCursorReferenceApiTest, ConstructAndDestructSingle)

REGISTER_REFERENCE_TEST (ExecuteDescendingEditCursorReferenceApiTest, MoveChain)

END_SUITE

BEGIN_SUITE (OrderedIndexQueries)

REGISTER_ALL_RANGE_QUERY_TESTS (TestQueryApiDrivers::CreateIndicesThanInsertRecords)

TEST_CASE (InsertBeforeCreation)
{
    TestQueryApiDrivers::InsertRecordsThanCreateIndices (Emergence::Query::Test::RangeQuery::SimpleLookups ());
}

END_SUITE
#include <Pegasus/Test/Scenario.hpp>

#include <Query/Test/DataTypes.hpp>
#include <Query/Test/RangeQueryTests.hpp>

#include <Reference/Test/Tests.hpp>

#include <StandardLayout/MappingBuilder.hpp>

#include <Testing/Testing.hpp>

using namespace Emergence::Pegasus::Test;

void ExecuteOrderedIndexReferenceApiTest (const Emergence::Reference::Test::Scenario &_scenario)
{
    std::vector <Task> tasks;
    tasks.emplace_back (CreateOrderedIndex {"source", Emergence::Query::Test::Player::Reflection::id});
    tasks += ReferenceApiTestImporters::ForIndexReference (_scenario, "source");
    tasks.emplace_back (DropIndex {"source"});
    Scenario (Emergence::Query::Test::Player::Reflection::GetMapping (), tasks);
}

BEGIN_SUITE (OrderedIndexReference)

REGISTER_ALL_REFERENCE_TESTS (ExecuteOrderedIndexReferenceApiTest)

END_SUITE

BEGIN_SUITE (OrderedIndexQueries)

REGISTER_ALL_RANGE_QUERY_TESTS (TestQueryApiDrivers::CreateIndicesThanInsertRecords)

TEST_CASE (InsertBeforeCreation)
{
    TestQueryApiDrivers::InsertRecordsThanCreateIndices (Emergence::Query::Test::RangeQuery::SimpleLookups ());
}

END_SUITE
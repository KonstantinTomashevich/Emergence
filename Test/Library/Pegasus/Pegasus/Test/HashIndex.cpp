#include <Pegasus/Test/Scenario.hpp>

#include <Query/Test/DataTypes.hpp>
#include <Query/Test/ValueQueryTests.hpp>

#include <Reference/Test/Tests.hpp>

#include <Testing/Testing.hpp>

using namespace Emergence::Pegasus::Test;

void ExecuteHashIndexReferenceApiTest (const Emergence::Reference::Test::Scenario &_scenario)
{
    std::vector <Task> tasks;
    tasks.emplace_back (CreateHashIndex {"source", {Emergence::Query::Test::Player::Reflection::id}});
    tasks += ReferenceApiTestImporters::ForIndexReference (_scenario, "source");
    tasks.emplace_back (DropIndex {"source"});
    Scenario (Emergence::Query::Test::Player::Reflection::GetMapping (), tasks);
}

BEGIN_SUITE (HashIndexReference)

REGISTER_ALL_REFERENCE_TESTS (ExecuteHashIndexReferenceApiTest)

END_SUITE

BEGIN_SUITE (HashIndexQueries)

REGISTER_ALL_VALUE_QUERY_TESTS (TestQueryApiDrivers::CreateIndicesThanInsertRecords)

TEST_CASE (InsertBeforeCreation)
{
    TestQueryApiDrivers::InsertRecordsThanCreateIndices (Emergence::Query::Test::ValueQuery::SimpleLookup ());
}

END_SUITE
#include <RecordCollection/Test/Scenario.hpp>
#include <RecordCollection/Test/PointRepresentation.hpp>

#include <Reference/Test/Tests.hpp>

#include <Query/Test/DataTypes.hpp>
#include <Query/Test/ValueQueryTests.hpp>

#include <Testing/Testing.hpp>

using namespace Emergence::RecordCollection::Test;

bool Emergence::RecordCollection::Test::PointRepresentationTestIncludeMarker () noexcept
{
    return true;
}

void ExecutePointRepresentationReferenceApiTest (const Emergence::Reference::Test::Scenario &_scenario)
{
    std::vector <Task> tasks;
    tasks.emplace_back (CreatePointRepresentation {"source", {Emergence::Query::Test::Player::Reflection::id}});
    tasks += ReferenceApiTestImporters::ForRepresentationReference (_scenario, "source");
    tasks.emplace_back (DropRepresentation {"source"});
    Scenario (Emergence::Query::Test::Player::Reflection::GetMapping (), tasks);
}

BEGIN_SUITE (PointRepresentationReference)

REGISTER_ALL_REFERENCE_TESTS (ExecutePointRepresentationReferenceApiTest)

END_SUITE

BEGIN_SUITE (PointRepresentationQueries)

REGISTER_ALL_VALUE_QUERY_TESTS (TestQueryApiDrivers::CreateRepresentationsThanAllocateRecords)

TEST_CASE (InsertBeforeCreation)
{
    TestQueryApiDrivers::AllocateRecordsThanCreateRepresentations (Emergence::Query::Test::ValueQuery::SimpleLookup ());
}

END_SUITE
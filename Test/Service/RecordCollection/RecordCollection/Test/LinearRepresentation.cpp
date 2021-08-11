#include <RecordCollection/Test/LinearRepresentation.hpp>
#include <RecordCollection/Test/Scenario.hpp>

#include <Reference/Test/Tests.hpp>

#include <Query/Test/DataTypes.hpp>
#include <Query/Test/RangeQueryTests.hpp>

#include <StandardLayout/MappingBuilder.hpp>

#include <Testing/Testing.hpp>

using namespace Emergence::RecordCollection::Test;

bool Emergence::RecordCollection::Test::LinearRepresentationTestIncludeMarker () noexcept
{
    return true;
}

void ExecuteLinearRepresentationReferenceApiTest (const Emergence::Reference::Test::Scenario &_scenario)
{
    std::vector <Task> tasks;
    tasks.emplace_back (CreateLinearRepresentation {"source", Emergence::Query::Test::Player::Reflection::id});
    tasks += ReferenceApiTestImporters::ForRepresentationReference (_scenario, "source");
    tasks.emplace_back (DropRepresentation {"source"});
    Scenario (Emergence::Query::Test::Player::Reflection::GetMapping (), tasks);
}

BEGIN_SUITE (LinearRepresentationReference)

REGISTER_ALL_REFERENCE_TESTS (ExecuteLinearRepresentationReferenceApiTest)

END_SUITE

BEGIN_SUITE (LinearRepresentationQueries)

REGISTER_ALL_RANGE_QUERY_TESTS (TestQueryApiDrivers::CreateRepresentationsThanAllocateRecords)

TEST_CASE (InsertBeforeCreation)
{
    TestQueryApiDrivers::AllocateRecordsThanCreateRepresentations (
        Emergence::Query::Test::RangeQuery::SimpleLookups ());
}

END_SUITE
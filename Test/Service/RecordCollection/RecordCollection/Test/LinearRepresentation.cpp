#include <RecordCollection/Test/Common.hpp>
#include <RecordCollection/Test/LinearRepresentation.hpp>
#include <RecordCollection/Test/Scenario.hpp>

#include <Query/Test/DataTypes.hpp>
#include <Query/Test/RangeQueryTests.hpp>

#include <StandardLayout/MappingBuilder.hpp>

#include <Testing/Testing.hpp>

using namespace Emergence::RecordCollection::Test;

bool Emergence::RecordCollection::Test::LinearRepresentationTestIncludeMarker () noexcept
{
    return true;
}

BEGIN_SUITE (LinearRepresentation)

TEST_CASE (ReferenceManipulations)
{
    Scenario {
        Emergence::Query::Test::Player::Reflection::GetMapping (),
        std::vector <Task>
            {
                CreateLinearRepresentation {"source", Emergence::Query::Test::Player::Reflection::id},
            } +
        Common::TestIsCanBeDropped ("source")
    };
}

REGISTER_ALL_RANGE_QUERY_TESTS (TestQueryApiDrivers::CreateRepresentationsThanAllocateRecords)

TEST_CASE (InsertBeforeCreation)
{
    TestQueryApiDrivers::AllocateRecordsThanCreateRepresentations (
        Emergence::Query::Test::RangeQuery::SimpleLookups ());
}

END_SUITE
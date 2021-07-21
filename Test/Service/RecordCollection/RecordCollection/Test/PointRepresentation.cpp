#include <RecordCollection/Test/Common.hpp>
#include <RecordCollection/Test/Scenario.hpp>
#include <RecordCollection/Test/PointRepresentation.hpp>

#include <Query/Test/DataTypes.hpp>
#include <Query/Test/ValueQueryTests.hpp>

#include <Testing/Testing.hpp>

using namespace Emergence::RecordCollection::Test;

bool Emergence::RecordCollection::Test::PointRepresentationTestIncludeMarker () noexcept
{
    return true;
}

BEGIN_SUITE (PointRepresentation)

TEST_CASE (ReferenceManipulations)
{
    Scenario
        {
            Emergence::Query::Test::Player::Reflection::GetMapping (),
            std::vector <Task>
                {
                    CreatePointRepresentation {"source", {Emergence::Query::Test::Player::Reflection::id}},
                } +
            Shortcuts::TestIsCanBeDropped ("source")
        };
}

REGISTER_ALL_VALUE_QUERY_TESTS (TestQueryApiDrivers::CreateRepresentationsThanAllocateRecords)

TEST_CASE (InsertBeforeCreation)
{
    TestQueryApiDrivers::AllocateRecordsThanCreateRepresentations (Emergence::Query::Test::ValueQuery::SimpleLookup ());
}

END_SUITE
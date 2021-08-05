#include <Pegasus/Test/Common.hpp>
#include <Pegasus/Test/Scenario.hpp>

#include <Query/Test/DataTypes.hpp>
#include <Query/Test/ValueQueryTests.hpp>

#include <Testing/Testing.hpp>

using namespace Emergence::Pegasus::Test;

BEGIN_SUITE (HashIndex)

TEST_CASE (ReferenceManipulations)
{
    Scenario
        {
            Emergence::Query::Test::Player::Reflection::GetMapping (),
            std::vector <Task>
                {
                    CreateHashIndex {"source", {Emergence::Query::Test::Player::Reflection::id}},
                } +
            Common::TestIsCanBeDropped ("source")
        };
}

REGISTER_ALL_VALUE_QUERY_TESTS (TestQueryApiDrivers::CreateIndicesThanInsertRecords)

TEST_CASE (InsertBeforeCreation)
{
    TestQueryApiDrivers::InsertRecordsThanCreateIndices (Emergence::Query::Test::ValueQuery::SimpleLookup ());
}

END_SUITE
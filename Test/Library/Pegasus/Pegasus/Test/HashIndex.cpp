#include <Pegasus/Test/Scenario.hpp>
#include <Pegasus/Test/Common.hpp>

#include <Query/Test/DataTypes.hpp>
#include <Query/Test/ValueQueryTests.hpp>

#include <Testing/Testing.hpp>

BEGIN_SUITE (HashIndex)

using namespace Emergence::Pegasus::Test;

TEST_CASE (ReferenceManipulations)
{
    Scenario
        {
            Emergence::Query::Test::Player::Reflection::GetMapping (),
            std::vector <Task>
                {
                    CreateHashIndex {"source", {Emergence::Query::Test::Player::Reflection::id}},
                } +
            Shortcuts::TestIsCanBeDropped ("source")
        };
}

REGISTER_ALL_VALUE_QUERY_TESTS (TestQueryApiDrivers::CreateIndicesThanInsertRecords)

TEST_CASE (InsertBeforeCreation)
{
    TestQueryApiDrivers::InsertRecordsThanCreateIndices (Emergence::Query::Test::ValueQuery::SimpleLookup ());
}

END_SUITE
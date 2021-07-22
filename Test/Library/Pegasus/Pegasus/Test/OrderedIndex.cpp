#include <Pegasus/Test/Common.hpp>
#include <Pegasus/Test/Scenario.hpp>

#include <Query/Test/DataTypes.hpp>
#include <Query/Test/RangeQueryTests.hpp>

#include <StandardLayout/MappingBuilder.hpp>

#include <Testing/Testing.hpp>

using namespace Emergence::Pegasus::Test;

BEGIN_SUITE (OrderedIndex)

TEST_CASE (ReferenceManipulations)
{
    Scenario {
        Emergence::Query::Test::Player::Reflection::GetMapping (),
        std::vector <Task>
            {
                CreateOrderedIndex {"source", Emergence::Query::Test::Player::Reflection::id},
            } +
        Shortcuts::TestIsCanBeDropped ("source")
    };
}

REGISTER_ALL_RANGE_QUERY_TESTS (TestQueryApiDrivers::CreateIndicesThanInsertRecords)

TEST_CASE (InsertBeforeCreation)
{
    TestQueryApiDrivers::InsertRecordsThanCreateIndices (Emergence::Query::Test::RangeQuery::SimpleLookups ());
}

END_SUITE
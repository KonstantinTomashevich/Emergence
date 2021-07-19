#include <Query/Test/Scenario.hpp>

#include <Testing/Testing.hpp>

namespace Emergence::Query::Test::AllQueryTypesInOneStorage
{
Scenario EditAndDeleteUsingValueQuery ();

Scenario EditAndDeleteUsingRangeQuery ();

Scenario EditAndDeleteUsingVolumetricQuery ();

// TODO: Unsupported: DropIndex
} // namespace Emergence::Query::Test::AllQueryTypesInOneStorage

#define REGISTER_ALL_QUERY_TYPES_IN_ONE_STORAGE_QUERY_TEST(Driver, TestName)                                           \
TEST_CASE (TestName)                                                                                                   \
{                                                                                                                      \
    Driver (Emergence::Query::Test::VolumetricQuery::TestName ());                                                     \
}                                                                                                                      \

#define REGISTER_ALL_TESTS_WITH_ALL_QUERY_TYPES_IN_ONE_STORAGE(Driver)                                                 \
REGISTER_ALL_QUERY_TYPES_IN_ONE_STORAGE_QUERY_TEST (Driver, EditAndDeleteUsingValueQuery)                              \
REGISTER_ALL_QUERY_TYPES_IN_ONE_STORAGE_QUERY_TEST (Driver, EditAndDeleteUsingRangeQuery)                              \
REGISTER_ALL_QUERY_TYPES_IN_ONE_STORAGE_QUERY_TEST (Driver, EditAndDeleteUsingVolumetricQuery)                         \

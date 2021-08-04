#include <Query/Test/Scenario.hpp>

#include <Testing/Testing.hpp>

// TODO: Think about renaming this suite after singleton and unordered sequence query additions.

namespace Emergence::Query::Test::AllQueryTypesInOneStorage
{
Scenario EditAndDeleteUsingValueQuery ();

Scenario EditAndDeleteUsingRangeQuery ();

Scenario EditAndDeleteUsingVolumetricQuery ();
} // namespace Emergence::Query::Test::AllQueryTypesInOneStorage

#define REGISTER_ALL_QUERY_TYPES_IN_ONE_STORAGE_QUERY_TEST(Driver, TestName)                                           \
TEST_CASE (TestName)                                                                                                   \
{                                                                                                                      \
    Driver (Emergence::Query::Test::AllQueryTypesInOneStorage::TestName ());                                                     \
}                                                                                                                      \

#define REGISTER_ALL_TESTS_WITH_ALL_QUERY_TYPES_IN_ONE_STORAGE(Driver)                                                 \
REGISTER_ALL_QUERY_TYPES_IN_ONE_STORAGE_QUERY_TEST (Driver, EditAndDeleteUsingValueQuery)                              \
REGISTER_ALL_QUERY_TYPES_IN_ONE_STORAGE_QUERY_TEST (Driver, EditAndDeleteUsingRangeQuery)                              \
REGISTER_ALL_QUERY_TYPES_IN_ONE_STORAGE_QUERY_TEST (Driver, EditAndDeleteUsingVolumetricQuery)                         \

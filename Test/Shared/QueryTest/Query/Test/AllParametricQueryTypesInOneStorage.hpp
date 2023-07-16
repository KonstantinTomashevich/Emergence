#include <Query/Test/Scenario.hpp>

#include <Testing/Testing.hpp>

namespace Emergence::Query::Test::AllParametricQueryTypesInOneStorage
{
Scenario EditAndDeleteUsingValueQuery ();

Scenario EditAndDeleteUsingRangeQuery ();

Scenario EditAndDeleteUsingVolumetricQuery ();
} // namespace Emergence::Query::Test::AllParametricQueryTypesInOneStorage

#define REGISTER_ALL_PARAMETRIC_QUERY_TYPES_IN_ONE_STORAGE_QUERY_TEST(Driver, TestName)                                \
    TEST_CASE (TestName)                                                                                               \
    {                                                                                                                  \
        Driver (Emergence::Query::Test::AllParametricQueryTypesInOneStorage::TestName ());                             \
    }

#define REGISTER_ALL_TESTS_WITH_ALL_PARAMETRIC_QUERY_TYPES_IN_ONE_STORAGE(Driver)                                      \
    REGISTER_ALL_PARAMETRIC_QUERY_TYPES_IN_ONE_STORAGE_QUERY_TEST (Driver, EditAndDeleteUsingValueQuery)               \
    REGISTER_ALL_PARAMETRIC_QUERY_TYPES_IN_ONE_STORAGE_QUERY_TEST (Driver, EditAndDeleteUsingRangeQuery)               \
    REGISTER_ALL_PARAMETRIC_QUERY_TYPES_IN_ONE_STORAGE_QUERY_TEST (Driver, EditAndDeleteUsingVolumetricQuery)

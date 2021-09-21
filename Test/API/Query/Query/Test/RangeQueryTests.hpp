#pragma once

#include <Query/Test/Scenario.hpp>

#include <Testing/Testing.hpp>

namespace Emergence::Query::Test::RangeQuery
{
Scenario SimpleLookups ();

Scenario OnStringField ();

Scenario WithDuplicates ();

Scenario Edition ();

Scenario Deletion ();

Scenario EditionAndDeletionFromDescendingCursor ();

Scenario MultipleSourcesEditionAndDeletion ();

Scenario OrderingInt8 ();

Scenario OrderingInt16 ();

Scenario OrderingInt32 ();

Scenario OrderingInt64 ();

Scenario OrderingUInt8 ();

Scenario OrderingUInt16 ();

Scenario OrderingUInt32 ();

Scenario OrderingUInt64 ();

Scenario OrderingFloat ();

Scenario OrderingDouble ();

Scenario OrderingBlock ();

Scenario OrderingString ();
} // namespace Emergence::Query::Test::RangeQuery

#define REGISTER_RANGE_QUERY_TEST(Driver, TestName)                                                                    \
    TEST_CASE (TestName)                                                                                               \
    {                                                                                                                  \
        Driver (Emergence::Query::Test::RangeQuery::TestName ());                                                      \
    }

#define REGISTER_ALL_RANGE_QUERY_TESTS(Driver)                                                                         \
    REGISTER_RANGE_QUERY_TEST (Driver, SimpleLookups)                                                                  \
    REGISTER_RANGE_QUERY_TEST (Driver, OnStringField)                                                                  \
    REGISTER_RANGE_QUERY_TEST (Driver, WithDuplicates)                                                                 \
    REGISTER_RANGE_QUERY_TEST (Driver, Edition)                                                                        \
    REGISTER_RANGE_QUERY_TEST (Driver, Deletion)                                                                       \
    REGISTER_RANGE_QUERY_TEST (Driver, EditionAndDeletionFromDescendingCursor)                                         \
    REGISTER_RANGE_QUERY_TEST (Driver, MultipleSourcesEditionAndDeletion)                                              \
    REGISTER_RANGE_QUERY_TEST (Driver, OrderingInt8)                                                                   \
    REGISTER_RANGE_QUERY_TEST (Driver, OrderingInt16)                                                                  \
    REGISTER_RANGE_QUERY_TEST (Driver, OrderingInt32)                                                                  \
    REGISTER_RANGE_QUERY_TEST (Driver, OrderingInt64)                                                                  \
    REGISTER_RANGE_QUERY_TEST (Driver, OrderingUInt8)                                                                  \
    REGISTER_RANGE_QUERY_TEST (Driver, OrderingUInt16)                                                                 \
    REGISTER_RANGE_QUERY_TEST (Driver, OrderingUInt32)                                                                 \
    REGISTER_RANGE_QUERY_TEST (Driver, OrderingUInt64)                                                                 \
    REGISTER_RANGE_QUERY_TEST (Driver, OrderingFloat)                                                                  \
    REGISTER_RANGE_QUERY_TEST (Driver, OrderingDouble)                                                                 \
    REGISTER_RANGE_QUERY_TEST (Driver, OrderingBlock)                                                                  \
    REGISTER_RANGE_QUERY_TEST (Driver, OrderingString)

#pragma once

#include <Query/Test/Scenario.hpp>

#include <Testing/Testing.hpp>

namespace Emergence::Query::Test::RangeQuery
{
Scenario CursorManipulations () noexcept;

Scenario ReversedCursorManipulations () noexcept;

Scenario SimpleLookups () noexcept;

Scenario OnStringField () noexcept;

Scenario WithDuplicates () noexcept;

Scenario Edition () noexcept;

Scenario Deletion () noexcept;

Scenario EditionAndDeletionFromReversedCursor () noexcept;

Scenario MultipleSourcesEditionAndDeletion () noexcept;

Scenario OrderingInt8 () noexcept;

Scenario OrderingInt16 () noexcept;

Scenario OrderingInt32 () noexcept;

Scenario OrderingInt64 () noexcept;

Scenario OrderingUInt8 () noexcept;

Scenario OrderingUInt16 () noexcept;

Scenario OrderingUInt32 () noexcept;

Scenario OrderingUInt64 () noexcept;

Scenario OrderingFloat () noexcept;

Scenario OrderingDouble () noexcept;

Scenario OrderingBlock () noexcept;

Scenario OrderingString () noexcept;
} // namespace Emergence::Query::Test::RangeQuery

#define REGISTER_RANGE_QUERY_TEST(Driver, TestName)                            \
TEST_CASE (TestName)                                                           \
{                                                                              \
    Driver (Emergence::Query::Test::RangeQuery::TestName ());                  \
}                                                                              \

#define REGISTER_ALL_RANGE_QUERY_TESTS(Driver)                                 \
REGISTER_RANGE_QUERY_TEST (Driver, CursorManipulations)                        \
REGISTER_RANGE_QUERY_TEST (Driver, ReversedCursorManipulations)                \
REGISTER_RANGE_QUERY_TEST (Driver, SimpleLookups)                              \
REGISTER_RANGE_QUERY_TEST (Driver, OnStringField)                              \
REGISTER_RANGE_QUERY_TEST (Driver, WithDuplicates)                             \
REGISTER_RANGE_QUERY_TEST (Driver, Edition)                                    \
REGISTER_RANGE_QUERY_TEST (Driver, Deletion)                                   \
REGISTER_RANGE_QUERY_TEST (Driver, EditionAndDeletionFromReversedCursor)       \
REGISTER_RANGE_QUERY_TEST (Driver, MultipleSourcesEditionAndDeletion)          \
REGISTER_RANGE_QUERY_TEST (Driver, OrderingInt8)                               \
REGISTER_RANGE_QUERY_TEST (Driver, OrderingInt16)                              \
REGISTER_RANGE_QUERY_TEST (Driver, OrderingInt32)                              \
REGISTER_RANGE_QUERY_TEST (Driver, OrderingInt64)                              \
REGISTER_RANGE_QUERY_TEST (Driver, OrderingUInt8)                              \
REGISTER_RANGE_QUERY_TEST (Driver, OrderingUInt16)                             \
REGISTER_RANGE_QUERY_TEST (Driver, OrderingUInt32)                             \
REGISTER_RANGE_QUERY_TEST (Driver, OrderingUInt64)                             \
REGISTER_RANGE_QUERY_TEST (Driver, OrderingFloat)                              \
REGISTER_RANGE_QUERY_TEST (Driver, OrderingDouble)                             \
REGISTER_RANGE_QUERY_TEST (Driver, OrderingBlock)                              \
REGISTER_RANGE_QUERY_TEST (Driver, OrderingString)                             \

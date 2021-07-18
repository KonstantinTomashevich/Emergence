#pragma once

#include <Query/Test/Scenario.hpp>

#include <Testing/Testing.hpp>

namespace Emergence::Query::Test::ValueQuery
{
// TODO: Unsupported: ReferenceManipulations

Scenario SimpleLookup () noexcept;

Scenario CursorManipulations () noexcept;

Scenario LookupForNonExistentRecord () noexcept;

// TODO: Unsupported: InsertBeforeCreation

Scenario LookupForMany () noexcept;

Scenario LookupAndEdit () noexcept;

Scenario OnStringField () noexcept;

Scenario OnTwoFields () noexcept;

Scenario OnBitField () noexcept;

Scenario OnTwoBitFields () noexcept;

Scenario MultipleIndicesEdition () noexcept;

Scenario MultipleIndicesDeletion () noexcept;
} // namespace Emergence::Query::Test::ValueQuery

#define REGISTER_VALUE_QUERY_TEST(Driver, TestName)                            \
TEST_CASE (TestName)                                                           \
{                                                                              \
    Driver (Emergence::Query::Test::ValueQuery::TestName ());                  \
}                                                                              \

#define REGISTER_ALL_VALUE_QUERY_TESTS(Driver)                                 \
REGISTER_VALUE_QUERY_TEST (Driver, SimpleLookup)                               \
REGISTER_VALUE_QUERY_TEST (Driver, CursorManipulations)                        \
REGISTER_VALUE_QUERY_TEST (Driver, LookupForNonExistentRecord)                 \
REGISTER_VALUE_QUERY_TEST (Driver, LookupForMany)                              \
REGISTER_VALUE_QUERY_TEST (Driver, LookupAndEdit)                              \
REGISTER_VALUE_QUERY_TEST (Driver, OnStringField)                              \
REGISTER_VALUE_QUERY_TEST (Driver, OnTwoFields)                                \
REGISTER_VALUE_QUERY_TEST (Driver, OnBitField)                                 \
REGISTER_VALUE_QUERY_TEST (Driver, OnTwoBitFields)                             \
REGISTER_VALUE_QUERY_TEST (Driver, MultipleIndicesEdition)                     \
REGISTER_VALUE_QUERY_TEST (Driver, MultipleIndicesDeletion)                    \

#pragma once

#include <Query/Test/Scenario.hpp>

#include <Testing/Testing.hpp>

namespace Emergence::Query::Test::UnorderedSequenceQuery
{
Scenario Read () noexcept;

Scenario Edit () noexcept;

Scenario Delete () noexcept;

Scenario EditAndDelete () noexcept;
} // namespace Emergence::Query::Test::UnorderedSequenceQuery

#define REGISTER_UNORDERED_SEQUENCE_QUERY_TEST(Driver, TestName)                                                       \
    TEST_CASE (TestName)                                                                                               \
    {                                                                                                                  \
        Driver (Emergence::Query::Test::UnorderedSequenceQuery::TestName ());                                          \
    }

#define REGISTER_ALL_UNORDERED_SEQUENCE_QUERY_TESTS(Driver)                                                            \
    REGISTER_UNORDERED_SEQUENCE_QUERY_TEST (Driver, Read)                                                              \
    REGISTER_UNORDERED_SEQUENCE_QUERY_TEST (Driver, Edit)                                                              \
    REGISTER_UNORDERED_SEQUENCE_QUERY_TEST (Driver, Delete)                                                            \
    REGISTER_UNORDERED_SEQUENCE_QUERY_TEST (Driver, EditAndDelete)

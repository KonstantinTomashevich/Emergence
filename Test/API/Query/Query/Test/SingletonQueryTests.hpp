#pragma once

#include <Query/Test/Scenario.hpp>

#include <Testing/Testing.hpp>

namespace Emergence::Query::Test::SingletonQuery
{
Scenario EditAndRead () noexcept;

Scenario CursorManipulations () noexcept;
} // namespace Emergence::Query::Test::SingletonQuery

#define REGISTER_SINGLETON_QUERY_TEST(Driver, TestName)                                                                \
    TEST_CASE (TestName)                                                                                               \
    {                                                                                                                  \
        Driver (Emergence::Query::Test::SingletonQuery::TestName ());                                                  \
    }

#define REGISTER_ALL_SINGLETON_QUERY_TESTS(Driver) REGISTER_SINGLETON_QUERY_TEST (Driver, EditAndRead)

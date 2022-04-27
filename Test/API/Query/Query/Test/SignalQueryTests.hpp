#pragma once

#include <Query/Test/Scenario.hpp>

#include <Testing/Testing.hpp>

namespace Emergence::Query::Test::SignalQuery
{
Scenario SimpleLookup ();

Scenario NoSignaled ();

Scenario MultipleSignaled ();

Scenario BitSignal ();

Scenario EditSignaled ();

Scenario DeleteSignaled ();

Scenario MultipleSourcesEdition ();

Scenario MultipleSourcesDeletion ();
} // namespace Emergence::Query::Test::SignalQuery

#define REGISTER_SIGNAL_QUERY_TEST(Driver, TestName)                                                                   \
    TEST_CASE (TestName)                                                                                               \
    {                                                                                                                  \
        Driver (Emergence::Query::Test::SignalQuery::TestName ());                                                     \
    }

#define REGISTER_ALL_SIGNAL_QUERY_TESTS(Driver)                                                                        \
    REGISTER_SIGNAL_QUERY_TEST (Driver, SimpleLookup)                                                                  \
    REGISTER_SIGNAL_QUERY_TEST (Driver, NoSignaled)                                                                    \
    REGISTER_SIGNAL_QUERY_TEST (Driver, MultipleSignaled)                                                              \
    REGISTER_SIGNAL_QUERY_TEST (Driver, BitSignal)                                                                     \
    REGISTER_SIGNAL_QUERY_TEST (Driver, EditSignaled)                                                                  \
    REGISTER_SIGNAL_QUERY_TEST (Driver, DeleteSignaled)                                                                \
    REGISTER_SIGNAL_QUERY_TEST (Driver, MultipleSourcesEdition)                                                        \
    REGISTER_SIGNAL_QUERY_TEST (Driver, MultipleSourcesDeletion)\

#pragma once

#include <Query/Test/Scenario.hpp>

#include <Testing/Testing.hpp>

namespace Emergence::Query::Test::ValueQuery
{
Scenario SimpleLookup ();

Scenario LookupForNonExistentObject ();

Scenario LookupForMany ();

Scenario LookupAndEdit ();

Scenario OnStringField ();

Scenario OnTwoFields ();

Scenario OnBitField ();

Scenario OnTwoBitFields ();

Scenario MultipleSourcesEdition ();

Scenario MultipleSourcesDeletion ();
} // namespace Emergence::Query::Test::ValueQuery

#define REGISTER_VALUE_QUERY_TEST(Driver, TestName)                                                                    \
    TEST_CASE (TestName)                                                                                               \
    {                                                                                                                  \
        Driver (Emergence::Query::Test::ValueQuery::TestName ());                                                      \
    }

#define REGISTER_ALL_VALUE_QUERY_TESTS(Driver)                                                                         \
    REGISTER_VALUE_QUERY_TEST (Driver, SimpleLookup)                                                                   \
    REGISTER_VALUE_QUERY_TEST (Driver, LookupForNonExistentObject)                                                     \
    REGISTER_VALUE_QUERY_TEST (Driver, LookupForMany)                                                                  \
    REGISTER_VALUE_QUERY_TEST (Driver, LookupAndEdit)                                                                  \
    REGISTER_VALUE_QUERY_TEST (Driver, OnStringField)                                                                  \
    REGISTER_VALUE_QUERY_TEST (Driver, OnTwoFields)                                                                    \
    REGISTER_VALUE_QUERY_TEST (Driver, OnBitField)                                                                     \
    REGISTER_VALUE_QUERY_TEST (Driver, OnTwoBitFields)                                                                 \
    REGISTER_VALUE_QUERY_TEST (Driver, MultipleSourcesEdition)                                                         \
    REGISTER_VALUE_QUERY_TEST (Driver, MultipleSourcesDeletion)

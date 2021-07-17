#pragma once

#include <Query/Test/Scenario.hpp>

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

#define REGISTER_ALL_VALUE_QUERY_TESTS(Driver)                                          \
TEST_CASE (SimpleLookup)                                                                \
{                                                                                       \
    Driver (Emergence::Query::Test::ValueQuery::SimpleLookup ());                       \
}                                                                                       \
                                                                                        \
TEST_CASE (CursorManipulations)                                                         \
{                                                                                       \
    Driver (Emergence::Query::Test::ValueQuery::CursorManipulations ());                \
}                                                                                       \
                                                                                        \
TEST_CASE (LookupForNonExistentRecord)                                                  \
{                                                                                       \
    Driver (Emergence::Query::Test::ValueQuery::LookupForNonExistentRecord ());         \
}                                                                                       \
                                                                                        \
TEST_CASE (LookupForMany)                                                               \
{                                                                                       \
    Driver (Emergence::Query::Test::ValueQuery::LookupForMany  ());                     \
}                                                                                       \
                                                                                        \
TEST_CASE (LookupAndEdit)                                                               \
{                                                                                       \
    Driver (Emergence::Query::Test::ValueQuery::LookupAndEdit());                       \
}                                                                                       \
                                                                                        \
TEST_CASE (OnStringField)                                                               \
{                                                                                       \
    Driver (Emergence::Query::Test::ValueQuery::OnStringField ());                      \
}                                                                                       \
                                                                                        \
TEST_CASE (OnTwoFields)                                                                 \
{                                                                                       \
    Driver (Emergence::Query::Test::ValueQuery::OnTwoFields ());                        \
}                                                                                       \
                                                                                        \
TEST_CASE (OnBitField)                                                                  \
{                                                                                       \
    Driver (Emergence::Query::Test::ValueQuery::OnBitField());                          \
}                                                                                       \
                                                                                        \
TEST_CASE (OnTwoBitFields)                                                              \
{                                                                                       \
    Driver (Emergence::Query::Test::ValueQuery::OnTwoBitFields ());                     \
}                                                                                       \
                                                                                        \
TEST_CASE (MultipleIndicesEdition)                                                      \
{                                                                                       \
    Driver (Emergence::Query::Test::ValueQuery::MultipleIndicesEdition ());             \
}                                                                                       \
                                                                                        \
TEST_CASE (MultipleIndicesDeletion)                                                     \
{                                                                                       \
    Driver (Emergence::Query::Test::ValueQuery::MultipleIndicesDeletion ());            \
}                                                                                       \

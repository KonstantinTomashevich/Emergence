#pragma once

#include <SyntaxSugar/BlockCast.hpp>

#define EMERGENCE_BIND_QUERY_COMMON_OPERATIONS(Query, QueryImplementation)                                             \
    Query::Query (const Query &_other) noexcept                                                                        \
    {                                                                                                                  \
        new (&data) QueryImplementation (block_cast<QueryImplementation> (_other.data));                               \
    }                                                                                                                  \
                                                                                                                       \
    Query::Query (Query &&_other) noexcept                                                                             \
    {                                                                                                                  \
        new (&data) QueryImplementation (std::move (block_cast<QueryImplementation> (_other.data)));                   \
    }                                                                                                                  \
                                                                                                                       \
    Query::~Query () noexcept                                                                                          \
    {                                                                                                                  \
        block_cast<QueryImplementation> (data).~QueryImplementation ();                                                \
    }                                                                                                                  \
                                                                                                                       \
    StandardLayout::Mapping Query::GetTypeMapping () const noexcept                                                    \
    {                                                                                                                  \
        return block_cast<QueryImplementation> (data).GetContainer ()->GetTypeMapping ();                              \
    }                                                                                                                  \
                                                                                                                       \
    Query::Query (std::array<uint8_t, DATA_MAX_SIZE> *_data) noexcept                                                  \
    {                                                                                                                  \
        new (&data) QueryImplementation (std::move (block_cast<QueryImplementation> (*_data)));                        \
    }

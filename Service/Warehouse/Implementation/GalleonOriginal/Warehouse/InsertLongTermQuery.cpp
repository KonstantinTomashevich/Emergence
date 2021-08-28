#include <Galleon/LongTermContainer.hpp>

#include <SyntaxSugar/BlockCast.hpp>

#include <Warehouse/InsertLongTermQuery.hpp>
#include <Warehouse/PreparedQueryBinding.hpp>

namespace Emergence::Warehouse
{
using QueryImplementation = Galleon::LongTermContainer::InsertQuery;

using CursorImplementation = QueryImplementation::Cursor;

InsertLongTermQuery::Cursor::Cursor (InsertLongTermQuery::Cursor &&_other) noexcept
{
    new (&data) CursorImplementation (std::move (block_cast<CursorImplementation> (_other.data)));
}

InsertLongTermQuery::Cursor::~Cursor () noexcept
{
    block_cast<CursorImplementation> (data).~CursorImplementation ();
}

void *InsertLongTermQuery::Cursor::operator++ () noexcept
{
    return ++block_cast<CursorImplementation> (data);
}

InsertLongTermQuery::Cursor::Cursor (std::array<uint8_t, DATA_MAX_SIZE> *_data) noexcept
{
    new (&data) CursorImplementation (std::move (block_cast<CursorImplementation> (*_data)));
}

EMERGENCE_BIND_QUERY_COMMON_OPERATIONS (InsertLongTermQuery, QueryImplementation)

InsertLongTermQuery::Cursor InsertLongTermQuery::Execute () noexcept
{
    CursorImplementation cursor = block_cast<QueryImplementation> (data).Execute ();
    return Cursor (reinterpret_cast<decltype (Cursor::data) *> (&cursor));
}
} // namespace Emergence::Warehouse
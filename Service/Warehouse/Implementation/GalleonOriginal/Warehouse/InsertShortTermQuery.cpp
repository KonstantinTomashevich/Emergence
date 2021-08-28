#include <Galleon/ShortTermContainer.hpp>

#include <SyntaxSugar/BlockCast.hpp>

#include <Warehouse/InsertShortTermQuery.hpp>
#include <Warehouse/PreparedQueryBinding.hpp>

namespace Emergence::Warehouse
{
using QueryImplementation = Galleon::ShortTermContainer::InsertQuery;

using CursorImplementation = QueryImplementation::Cursor;

InsertShortTermQuery::Cursor::Cursor (InsertShortTermQuery::Cursor &&_other) noexcept
{
    new (&data) CursorImplementation (std::move (block_cast<CursorImplementation> (_other.data)));
}

InsertShortTermQuery::Cursor::~Cursor () noexcept
{
    block_cast<CursorImplementation> (data).~CursorImplementation ();
}

void *InsertShortTermQuery::Cursor::operator++ () noexcept
{
    return ++block_cast<CursorImplementation> (data);
}

InsertShortTermQuery::Cursor::Cursor (std::array<uint8_t, DATA_MAX_SIZE> *_data) noexcept
{
    new (&data) CursorImplementation (std::move (block_cast<CursorImplementation> (*_data)));
}

EMERGENCE_BIND_QUERY_COMMON_OPERATIONS (InsertShortTermQuery, QueryImplementation)

InsertShortTermQuery::Cursor InsertShortTermQuery::Execute () noexcept
{
    CursorImplementation cursor = block_cast<QueryImplementation> (data).Execute ();
    return Cursor (reinterpret_cast<decltype (Cursor::data) *> (&cursor));
}
} // namespace Emergence::Warehouse

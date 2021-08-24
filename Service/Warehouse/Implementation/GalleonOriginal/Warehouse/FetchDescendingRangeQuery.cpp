#include <API/Common/Implementation/Cursor.hpp>

#include <Galleon/LongTermContainer.hpp>

#include <SyntaxSugar/BlockCast.hpp>

#include <Warehouse/FetchDescendingRangeQuery.hpp>
#include <Warehouse/PreparedQueryBinding.hpp>

namespace Emergence::Warehouse
{
using Cursor = FetchDescendingRangeQuery::Cursor;

using QueryImplementation = Galleon::LongTermContainer::FetchDescendingRangeQuery;

using CursorImplementation = QueryImplementation::Cursor;

EMERGENCE_BIND_READ_CURSOR_OPERATIONS_IMPLEMENTATION (Cursor, CursorImplementation)

EMERGENCE_BIND_QUERY_COMMON_OPERATIONS (FetchDescendingRangeQuery, QueryImplementation)

Cursor FetchDescendingRangeQuery::Execute (const Bound _min, const Bound _max) noexcept
{
    CursorImplementation cursor = block_cast <QueryImplementation> (data).Execute (_min, _max);
    return Cursor (reinterpret_cast <decltype (Cursor::data) *> (&cursor));
}

StandardLayout::Field FetchDescendingRangeQuery::GetKeyField () const noexcept
{
    return block_cast <QueryImplementation> (data).GetKeyField ();
}
} // namespace Emergence::Warehouse
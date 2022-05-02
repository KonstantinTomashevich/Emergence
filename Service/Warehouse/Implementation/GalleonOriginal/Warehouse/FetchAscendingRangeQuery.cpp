#include <API/Common/Implementation/Cursor.hpp>

#include <Galleon/LongTermContainer.hpp>

#include <SyntaxSugar/BlockCast.hpp>

#include <Warehouse/FetchAscendingRangeQuery.hpp>
#include <Warehouse/PreparedQueryBinding.hpp>

namespace Emergence::Warehouse
{
using Cursor = FetchAscendingRangeQuery::Cursor;

using QueryImplementation = Galleon::LongTermContainer::FetchAscendingRangeQuery;

using CursorImplementation = QueryImplementation::Cursor;

EMERGENCE_BIND_READ_CURSOR_OPERATIONS_IMPLEMENTATION (Cursor, CursorImplementation)

EMERGENCE_BIND_QUERY_COMMON_OPERATIONS (FetchAscendingRangeQuery, QueryImplementation)

Cursor FetchAscendingRangeQuery::Execute (Bound _min, Bound _max) noexcept
{
    CursorImplementation cursor = block_cast<QueryImplementation> (data).Execute (_min, _max);
    return Cursor (array_cast (cursor));
}

StandardLayout::Field FetchAscendingRangeQuery::GetKeyField () const noexcept
{
    return block_cast<QueryImplementation> (data).GetKeyField ();
}
} // namespace Emergence::Warehouse

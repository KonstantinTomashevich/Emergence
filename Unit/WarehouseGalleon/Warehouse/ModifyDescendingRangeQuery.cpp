#include <API/Common/BlockCast.hpp>
#include <API/Common/Implementation/Cursor.hpp>

#include <Galleon/LongTermContainer.hpp>

#include <Warehouse/ModifyDescendingRangeQuery.hpp>
#include <Warehouse/PreparedQueryBinding.hpp>

namespace Emergence::Warehouse
{
using Cursor = ModifyDescendingRangeQuery::Cursor;

using QueryImplementation = Galleon::LongTermContainer::ModifyDescendingRangeQuery;

using CursorImplementation = QueryImplementation::Cursor;

EMERGENCE_BIND_EDIT_CURSOR_OPERATIONS_IMPLEMENTATION (Cursor, CursorImplementation)

EMERGENCE_BIND_MODIFY_QUERY_COMMON_OPERATIONS (ModifyDescendingRangeQuery, QueryImplementation)

Cursor ModifyDescendingRangeQuery::Execute (Bound _min, Bound _max) noexcept
{
    CursorImplementation cursor = block_cast<QueryImplementation> (data).Execute (_min, _max);
    return Cursor (array_cast (cursor));
}

StandardLayout::Field ModifyDescendingRangeQuery::GetKeyField () const noexcept
{
    return block_cast<QueryImplementation> (data).GetKeyField ();
}
} // namespace Emergence::Warehouse

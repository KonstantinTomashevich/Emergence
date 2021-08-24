#include <API/Common/Implementation/Cursor.hpp>

#include <Galleon/LongTermContainer.hpp>

#include <SyntaxSugar/BlockCast.hpp>

#include <Warehouse/ModifyAscendingRangeQuery.hpp>
#include <Warehouse/PreparedQueryBinding.hpp>

namespace Emergence::Warehouse
{
using Cursor = ModifyAscendingRangeQuery::Cursor;

using QueryImplementation = Galleon::LongTermContainer::ModifyAscendingRangeQuery;

using CursorImplementation = QueryImplementation::Cursor;

EMERGENCE_BIND_EDIT_CURSOR_OPERATIONS_IMPLEMENTATION (Cursor, CursorImplementation)

EMERGENCE_BIND_QUERY_COMMON_OPERATIONS (ModifyAscendingRangeQuery, QueryImplementation)

Cursor ModifyAscendingRangeQuery::Execute (const Bound _min, const Bound _max) noexcept
{
    CursorImplementation cursor = block_cast <QueryImplementation> (data).Execute (_min, _max);
    return Cursor (reinterpret_cast <decltype (Cursor::data) *> (&cursor));
}

StandardLayout::Field ModifyAscendingRangeQuery::GetKeyField () const noexcept
{
    return block_cast <QueryImplementation> (data).GetKeyField ();
}
} // namespace Emergence::Warehouse
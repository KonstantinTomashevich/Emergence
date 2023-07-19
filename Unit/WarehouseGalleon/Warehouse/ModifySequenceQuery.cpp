#include <API/Common/BlockCast.hpp>
#include <API/Common/Implementation/Cursor.hpp>

#include <Galleon/ShortTermContainer.hpp>

#include <Warehouse/ModifySequenceQuery.hpp>
#include <Warehouse/PreparedQueryBinding.hpp>

namespace Emergence::Warehouse
{
using Cursor = ModifySequenceQuery::Cursor;

using QueryImplementation = Galleon::ShortTermContainer::ModifyQuery;

using CursorImplementation = QueryImplementation::Cursor;

EMERGENCE_BIND_EDIT_CURSOR_OPERATIONS_IMPLEMENTATION (Cursor, CursorImplementation)

EMERGENCE_BIND_MODIFY_QUERY_COMMON_OPERATIONS (ModifySequenceQuery, QueryImplementation)

Cursor ModifySequenceQuery::Execute () noexcept
{
    CursorImplementation cursor = block_cast<QueryImplementation> (data).Execute ();
    return Cursor (array_cast (cursor));
}
} // namespace Emergence::Warehouse

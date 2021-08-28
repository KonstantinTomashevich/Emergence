#include <API/Common/Implementation/Cursor.hpp>

#include <Galleon/ShortTermContainer.hpp>

#include <SyntaxSugar/BlockCast.hpp>

#include <Warehouse/FetchSequenceQuery.hpp>
#include <Warehouse/PreparedQueryBinding.hpp>

namespace Emergence::Warehouse
{
using Cursor = FetchSequenceQuery::Cursor;

using QueryImplementation = Galleon::ShortTermContainer::FetchQuery;

using CursorImplementation = QueryImplementation::Cursor;

EMERGENCE_BIND_READ_CURSOR_OPERATIONS_IMPLEMENTATION (Cursor, CursorImplementation)

EMERGENCE_BIND_QUERY_COMMON_OPERATIONS (FetchSequenceQuery, QueryImplementation)

Cursor FetchSequenceQuery::Execute () noexcept
{
    CursorImplementation cursor = block_cast<QueryImplementation> (data).Execute ();
    return Cursor (reinterpret_cast<decltype (Cursor::data) *> (&cursor));
}
} // namespace Emergence::Warehouse

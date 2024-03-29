#include <API/Common/BlockCast.hpp>
#include <API/Common/Implementation/Cursor.hpp>

#include <Galleon/LongTermContainer.hpp>

#include <Warehouse/FetchValueQuery.hpp>
#include <Warehouse/PreparedQueryBinding.hpp>

namespace Emergence::Warehouse
{
using Cursor = FetchValueQuery::Cursor;

using QueryImplementation = Galleon::LongTermContainer::FetchValueQuery;

using CursorImplementation = QueryImplementation::Cursor;

EMERGENCE_BIND_READ_CURSOR_OPERATIONS_IMPLEMENTATION (Cursor, CursorImplementation)

EMERGENCE_BIND_QUERY_COMMON_OPERATIONS (FetchValueQuery, QueryImplementation)

Cursor FetchValueQuery::Execute (ValueSequence _values) noexcept
{
    CursorImplementation cursor = block_cast<QueryImplementation> (data).Execute (_values);
    return Cursor (array_cast (cursor));
}

KeyFieldIterator FetchValueQuery::KeyFieldBegin () const noexcept
{
    auto iterator = block_cast<QueryImplementation> (data).KeyFieldBegin ();
    return KeyFieldIterator (array_cast (iterator));
}

KeyFieldIterator FetchValueQuery::KeyFieldEnd () const noexcept
{
    auto iterator = block_cast<QueryImplementation> (data).KeyFieldEnd ();
    return KeyFieldIterator (array_cast (iterator));
}
} // namespace Emergence::Warehouse

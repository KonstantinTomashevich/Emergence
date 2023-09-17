#include <API/Common/BlockCast.hpp>
#include <API/Common/Implementation/Cursor.hpp>

#include <Galleon/LongTermContainer.hpp>

#include <Warehouse/ModifyValueQuery.hpp>
#include <Warehouse/PreparedQueryBinding.hpp>

namespace Emergence::Warehouse
{
using Cursor = ModifyValueQuery::Cursor;

using QueryImplementation = Galleon::LongTermContainer::ModifyValueQuery;

using CursorImplementation = QueryImplementation::Cursor;

EMERGENCE_BIND_EDIT_CURSOR_OPERATIONS_IMPLEMENTATION (Cursor, CursorImplementation)

EMERGENCE_BIND_MODIFY_QUERY_COMMON_OPERATIONS (ModifyValueQuery, QueryImplementation)

Cursor ModifyValueQuery::Execute (ValueSequence _values) noexcept
{
    CursorImplementation cursor = block_cast<QueryImplementation> (data).Execute (_values);
    return Cursor (array_cast (cursor));
}

KeyFieldIterator ModifyValueQuery::KeyFieldBegin () const noexcept
{
    auto iterator = block_cast<QueryImplementation> (data).KeyFieldBegin ();
    return KeyFieldIterator (array_cast (iterator));
}

KeyFieldIterator ModifyValueQuery::KeyFieldEnd () const noexcept
{
    auto iterator = block_cast<QueryImplementation> (data).KeyFieldEnd ();
    return KeyFieldIterator (array_cast (iterator));
}
} // namespace Emergence::Warehouse

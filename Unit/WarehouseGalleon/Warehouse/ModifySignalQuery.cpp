#include <API/Common/Implementation/Cursor.hpp>

#include <Galleon/LongTermContainer.hpp>

#include <SyntaxSugar/BlockCast.hpp>

#include <Warehouse/ModifySignalQuery.hpp>
#include <Warehouse/PreparedQueryBinding.hpp>

namespace Emergence::Warehouse
{
using Cursor = ModifySignalQuery::Cursor;

using QueryImplementation = Galleon::LongTermContainer::ModifySignalQuery;

using CursorImplementation = QueryImplementation::Cursor;

EMERGENCE_BIND_EDIT_CURSOR_OPERATIONS_IMPLEMENTATION (Cursor, CursorImplementation)

EMERGENCE_BIND_MODIFY_QUERY_COMMON_OPERATIONS (ModifySignalQuery, QueryImplementation)

Cursor ModifySignalQuery::Execute () noexcept
{
    CursorImplementation cursor = block_cast<QueryImplementation> (data).Execute ();
    return Cursor (array_cast (cursor));
}

StandardLayout::Field ModifySignalQuery::GetKeyField () const noexcept
{
    return block_cast<QueryImplementation> (data).GetKeyField ();
}

bool ModifySignalQuery::IsSignaledValue (const std::array<std::uint8_t, sizeof (std::uint64_t)> &_value) const
{
    return block_cast<QueryImplementation> (data).IsSignaledValue (_value);
}

std::array<std::uint8_t, sizeof (std::uint64_t)> ModifySignalQuery::GetSignaledValue () const noexcept
{
    return block_cast<QueryImplementation> (data).GetSignaledValue ();
}
} // namespace Emergence::Warehouse

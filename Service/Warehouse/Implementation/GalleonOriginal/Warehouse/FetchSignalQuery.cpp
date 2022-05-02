#include <API/Common/Implementation/Cursor.hpp>

#include <Galleon/LongTermContainer.hpp>

#include <SyntaxSugar/BlockCast.hpp>

#include <Warehouse/FetchSignalQuery.hpp>
#include <Warehouse/PreparedQueryBinding.hpp>

namespace Emergence::Warehouse
{
using Cursor = FetchSignalQuery::Cursor;

using QueryImplementation = Galleon::LongTermContainer::FetchSignalQuery;

using CursorImplementation = QueryImplementation::Cursor;

EMERGENCE_BIND_READ_CURSOR_OPERATIONS_IMPLEMENTATION (Cursor, CursorImplementation)

EMERGENCE_BIND_QUERY_COMMON_OPERATIONS (FetchSignalQuery, QueryImplementation)

Cursor FetchSignalQuery::Execute () noexcept
{
    CursorImplementation cursor = block_cast<QueryImplementation> (data).Execute ();
    return Cursor (array_cast (cursor));
}

StandardLayout::Field FetchSignalQuery::GetKeyField () const noexcept
{
    return block_cast<QueryImplementation> (data).GetKeyField ();
}

bool FetchSignalQuery::IsSignaledValue (const std::array<uint8_t, sizeof (uint64_t)> &_value) const
{
    return block_cast<QueryImplementation> (data).IsSignaledValue (_value);
}

std::array<uint8_t, sizeof (uint64_t)> FetchSignalQuery::GetSignaledValue () const noexcept
{
    return block_cast<QueryImplementation> (data).GetSignaledValue ();
}
} // namespace Emergence::Warehouse

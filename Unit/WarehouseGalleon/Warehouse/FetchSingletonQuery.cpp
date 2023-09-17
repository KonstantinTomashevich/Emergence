#include <API/Common/BlockCast.hpp>

#include <Galleon/SingletonContainer.hpp>

#include <Warehouse/FetchSingletonQuery.hpp>
#include <Warehouse/PreparedQueryBinding.hpp>

namespace Emergence::Warehouse
{
using QueryImplementation = Galleon::SingletonContainer::FetchQuery;

using CursorImplementation = QueryImplementation::Cursor;

FetchSingletonQuery::Cursor::Cursor (const FetchSingletonQuery::Cursor &_other) noexcept
{
    new (&data) CursorImplementation (block_cast<CursorImplementation> (_other.data));
}

FetchSingletonQuery::Cursor::Cursor (FetchSingletonQuery::Cursor &&_other) noexcept
{
    new (&data) CursorImplementation (std::move (block_cast<CursorImplementation> (_other.data)));
}

FetchSingletonQuery::Cursor::~Cursor () noexcept
{
    block_cast<CursorImplementation> (data).~CursorImplementation ();
}

const void *FetchSingletonQuery::Cursor::operator* () const noexcept
{
    return *block_cast<CursorImplementation> (data);
}

FetchSingletonQuery::Cursor::Cursor (std::array<std::uint8_t, DATA_MAX_SIZE> &_data) noexcept
{
    new (&data) CursorImplementation (std::move (block_cast<CursorImplementation> (_data)));
}

EMERGENCE_BIND_QUERY_COMMON_OPERATIONS (FetchSingletonQuery, QueryImplementation)

FetchSingletonQuery::Cursor FetchSingletonQuery::Execute () noexcept
{
    CursorImplementation cursor = block_cast<QueryImplementation> (data).Execute ();
    return Cursor (array_cast (cursor));
}
} // namespace Emergence::Warehouse

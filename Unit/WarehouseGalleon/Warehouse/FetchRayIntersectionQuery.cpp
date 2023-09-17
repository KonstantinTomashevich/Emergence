#include <API/Common/BlockCast.hpp>
#include <API/Common/Implementation/Cursor.hpp>

#include <Galleon/LongTermContainer.hpp>

#include <Warehouse/FetchRayIntersectionQuery.hpp>
#include <Warehouse/PreparedQueryBinding.hpp>

namespace Emergence::Warehouse
{
using Cursor = FetchRayIntersectionQuery::Cursor;

using QueryImplementation = Galleon::LongTermContainer::FetchRayIntersectionQuery;

using CursorImplementation = QueryImplementation::Cursor;

EMERGENCE_BIND_READ_CURSOR_OPERATIONS_IMPLEMENTATION (Cursor, CursorImplementation)

EMERGENCE_BIND_QUERY_COMMON_OPERATIONS (FetchRayIntersectionQuery, QueryImplementation)

Cursor FetchRayIntersectionQuery::Execute (Ray _ray, float _maxDistance) noexcept
{
    CursorImplementation cursor = block_cast<QueryImplementation> (data).Execute (_ray, _maxDistance);
    return Cursor (array_cast (cursor));
}

DimensionIterator FetchRayIntersectionQuery::DimensionBegin () const noexcept
{
    auto iterator = block_cast<QueryImplementation> (data).DimensionBegin ();
    return DimensionIterator (array_cast (iterator));
}

DimensionIterator FetchRayIntersectionQuery::DimensionEnd () const noexcept
{
    auto iterator = block_cast<QueryImplementation> (data).DimensionEnd ();
    return DimensionIterator (array_cast (iterator));
}
} // namespace Emergence::Warehouse

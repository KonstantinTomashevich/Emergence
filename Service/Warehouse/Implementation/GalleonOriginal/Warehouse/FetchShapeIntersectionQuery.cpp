#include <API/Common/Implementation/Cursor.hpp>

#include <Galleon/LongTermContainer.hpp>

#include <SyntaxSugar/BlockCast.hpp>

#include <Warehouse/FetchShapeIntersectionQuery.hpp>
#include <Warehouse/PreparedQueryBinding.hpp>

namespace Emergence::Warehouse
{
using Cursor = FetchShapeIntersectionQuery::Cursor;

using QueryImplementation = Galleon::LongTermContainer::FetchShapeIntersectionQuery;

using CursorImplementation = QueryImplementation::Cursor;

EMERGENCE_BIND_READ_CURSOR_OPERATIONS_IMPLEMENTATION (Cursor, CursorImplementation)

EMERGENCE_BIND_QUERY_COMMON_OPERATIONS (FetchShapeIntersectionQuery, QueryImplementation)

Cursor FetchShapeIntersectionQuery::Execute (Shape _shape) noexcept
{
    CursorImplementation cursor = block_cast<QueryImplementation> (data).Execute (_shape);
    return Cursor (array_cast (cursor));
}

DimensionIterator FetchShapeIntersectionQuery::DimensionBegin () const noexcept
{
    auto iterator = block_cast<QueryImplementation> (data).DimensionBegin ();
    return DimensionIterator (array_cast (iterator));
}

DimensionIterator FetchShapeIntersectionQuery::DimensionEnd () const noexcept
{
    auto iterator = block_cast<QueryImplementation> (data).DimensionEnd ();
    return DimensionIterator (array_cast (iterator));
}
} // namespace Emergence::Warehouse

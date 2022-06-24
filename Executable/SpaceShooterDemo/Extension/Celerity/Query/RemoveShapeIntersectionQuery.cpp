#include <Celerity/Query/RemoveShapeIntersectionQuery.hpp>

namespace Emergence::Celerity
{
EMERGENCE_CELERITY_REMOVE_QUERY_WRAPPER_IMPLEMENTATION (ShapeIntersectionQuery)

RemoveShapeIntersectionQuery::Cursor RemoveShapeIntersectionQuery::Execute (Warehouse::Shape _shape) noexcept
{
    return Cursor {source.Execute (_shape), eventsOnRemove, changeTracker};
}
} // namespace Emergence::Celerity

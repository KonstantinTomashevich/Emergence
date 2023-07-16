#include <Celerity/Query/EditShapeIntersectionQuery.hpp>

namespace Emergence::Celerity
{
EMERGENCE_CELERITY_EDIT_QUERY_WRAPPER_IMPLEMENTATION (ShapeIntersectionQuery)

EditShapeIntersectionQuery::Cursor EditShapeIntersectionQuery::Execute (Warehouse::Shape _shape) noexcept
{
    return Cursor {source.Execute (_shape), eventsOnRemove, eventsOnChange, changeTracker};
}
} // namespace Emergence::Celerity

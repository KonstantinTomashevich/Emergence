#include <Celerity/Query/ModifyShapeIntersectionQuery.hpp>

namespace Emergence::Celerity
{
WAREHOUSE_MODIFY_QUERY_WRAPPER_IMPLEMENTATION (ModifyShapeIntersectionQuery)

ModifyShapeIntersectionQuery::Cursor ModifyShapeIntersectionQuery::Execute (Warehouse::Shape _shape) noexcept
{
    return Cursor {source.Execute (_shape), eventsOnRemove, changeTracker};
}
} // namespace Emergence::Celerity

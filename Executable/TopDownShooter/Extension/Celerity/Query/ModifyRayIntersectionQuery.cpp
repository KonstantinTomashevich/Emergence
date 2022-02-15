#include <Celerity/Query/ModifyRayIntersectionQuery.hpp>

namespace Emergence::Celerity
{
WAREHOUSE_MODIFY_QUERY_WRAPPER_IMPLEMENTATION (ModifyRayIntersectionQuery)

ModifyRayIntersectionQuery::Cursor ModifyRayIntersectionQuery::Execute (Warehouse::Ray _ray,
                                                                        float _maxDistance) noexcept
{
    return Cursor {source.Execute (_ray, _maxDistance), eventsOnRemove, changeTracker};
}
} // namespace Emergence::Celerity

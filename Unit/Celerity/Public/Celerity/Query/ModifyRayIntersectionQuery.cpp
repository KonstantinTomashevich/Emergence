#include <Celerity/Query/ModifyRayIntersectionQuery.hpp>

namespace Emergence::Celerity
{
EMERGENCE_CELERITY_MODIFY_QUERY_WRAPPER_IMPLEMENTATION (RayIntersectionQuery)

ModifyRayIntersectionQuery::Cursor ModifyRayIntersectionQuery::Execute (Warehouse::Ray _ray,
                                                                        float _maxDistance) noexcept
{
    return Cursor {source.Execute (_ray, _maxDistance), eventsOnRemove, eventsOnChange, changeTracker};
}
} // namespace Emergence::Celerity

#include <Celerity/Query/RemoveRayIntersectionQuery.hpp>

namespace Emergence::Celerity
{
EMERGENCE_CELERITY_REMOVE_QUERY_WRAPPER_IMPLEMENTATION (RayIntersectionQuery)

RemoveRayIntersectionQuery::Cursor RemoveRayIntersectionQuery::Execute (Warehouse::Ray _ray,
                                                                        float _maxDistance) noexcept
{
    return Cursor {source.Execute (_ray, _maxDistance), eventsOnRemove, changeTracker};
}
} // namespace Emergence::Celerity

#include <Celerity/Query/EditRayIntersectionQuery.hpp>

namespace Emergence::Celerity
{
EMERGENCE_CELERITY_EDIT_QUERY_WRAPPER_IMPLEMENTATION (RayIntersectionQuery)

EditRayIntersectionQuery::Cursor EditRayIntersectionQuery::Execute (Warehouse::Ray _ray, float _maxDistance) noexcept
{
    return Cursor {source.Execute (_ray, _maxDistance), eventsOnRemove, changeTracker};
}
} // namespace Emergence::Celerity

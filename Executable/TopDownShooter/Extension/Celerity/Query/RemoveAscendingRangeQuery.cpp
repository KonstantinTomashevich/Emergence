#include <Celerity/Query/RemoveAscendingRangeQuery.hpp>

namespace Emergence::Celerity
{
EMERGENCE_CELERITY_REMOVE_QUERY_WRAPPER_IMPLEMENTATION (AscendingRangeQuery)

RemoveAscendingRangeQuery::Cursor RemoveAscendingRangeQuery::Execute (Warehouse::Bound _min,
                                                                      Warehouse::Bound _max) noexcept
{
    return Cursor {source.Execute (_min, _max), eventsOnRemove, changeTracker};
}
} // namespace Emergence::Celerity

#include <Celerity/Query/RemoveDescendingRangeQuery.hpp>

namespace Emergence::Celerity
{
EMERGENCE_CELERITY_REMOVE_QUERY_WRAPPER_IMPLEMENTATION (DescendingRangeQuery)

RemoveDescendingRangeQuery::Cursor RemoveDescendingRangeQuery::Execute (Warehouse::Bound _min,
                                                                        Warehouse::Bound _max) noexcept
{
    return Cursor {source.Execute (_min, _max), eventsOnRemove, changeTracker};
}
} // namespace Emergence::Celerity

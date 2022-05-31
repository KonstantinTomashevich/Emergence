#include <Celerity/Query/EditDescendingRangeQuery.hpp>

namespace Emergence::Celerity
{
EMERGENCE_CELERITY_EDIT_QUERY_WRAPPER_IMPLEMENTATION (DescendingRangeQuery)

EditDescendingRangeQuery::Cursor EditDescendingRangeQuery::Execute (Warehouse::Bound _min,
                                                                    Warehouse::Bound _max) noexcept
{
    return Cursor {source.Execute (_min, _max), eventsOnRemove, changeTracker};
}
} // namespace Emergence::Celerity

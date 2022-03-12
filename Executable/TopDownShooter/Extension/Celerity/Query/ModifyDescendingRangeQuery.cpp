#include <Celerity/Query/ModifyDescendingRangeQuery.hpp>

namespace Emergence::Celerity
{
EMERGENCE_CELERITY_MODIFY_QUERY_WRAPPER_IMPLEMENTATION (ModifyDescendingRangeQuery)

ModifyDescendingRangeQuery::Cursor ModifyDescendingRangeQuery::Execute (Warehouse::Bound _min,
                                                                        Warehouse::Bound _max) noexcept
{
    return Cursor {source.Execute (_min, _max), eventsOnRemove, changeTracker};
}
} // namespace Emergence::Celerity

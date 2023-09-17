#include <Celerity/Query/ModifyDescendingRangeQuery.hpp>

namespace Emergence::Celerity
{
EMERGENCE_CELERITY_MODIFY_QUERY_WRAPPER_IMPLEMENTATION (DescendingRangeQuery)

ModifyDescendingRangeQuery::Cursor ModifyDescendingRangeQuery::Execute (Warehouse::Bound _min,
                                                                        Warehouse::Bound _max) noexcept
{
    return Cursor {source.Execute (_min, _max), eventsOnRemove, eventsOnChange, changeTracker};
}
} // namespace Emergence::Celerity

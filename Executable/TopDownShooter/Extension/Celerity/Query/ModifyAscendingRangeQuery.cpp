#include <Celerity/Query/ModifyAscendingRangeQuery.hpp>

namespace Emergence::Celerity
{
EMERGENCE_CELERITY_MODIFY_QUERY_WRAPPER_IMPLEMENTATION (ModifyAscendingRangeQuery)

ModifyAscendingRangeQuery::Cursor ModifyAscendingRangeQuery::Execute (Warehouse::Bound _min,
                                                                      Warehouse::Bound _max) noexcept
{
    return Cursor {source.Execute (_min, _max), eventsOnRemove, changeTracker};
}
} // namespace Emergence::Celerity

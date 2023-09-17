#include <Celerity/Query/ModifyAscendingRangeQuery.hpp>

namespace Emergence::Celerity
{
EMERGENCE_CELERITY_MODIFY_QUERY_WRAPPER_IMPLEMENTATION (AscendingRangeQuery)

ModifyAscendingRangeQuery::Cursor ModifyAscendingRangeQuery::Execute (Warehouse::Bound _min,
                                                                      Warehouse::Bound _max) noexcept
{
    return Cursor {source.Execute (_min, _max), eventsOnRemove, eventsOnChange, changeTracker};
}
} // namespace Emergence::Celerity

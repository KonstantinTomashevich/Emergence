#include <Celerity/Query/EditAscendingRangeQuery.hpp>

namespace Emergence::Celerity
{
EMERGENCE_CELERITY_EDIT_QUERY_WRAPPER_IMPLEMENTATION (AscendingRangeQuery)

EditAscendingRangeQuery::Cursor EditAscendingRangeQuery::Execute (Warehouse::Bound _min, Warehouse::Bound _max) noexcept
{
    return Cursor {source.Execute (_min, _max), eventsOnRemove, eventsOnChange, changeTracker};
}
} // namespace Emergence::Celerity

#include <Celerity/Query/EditValueQuery.hpp>

namespace Emergence::Celerity
{
EMERGENCE_CELERITY_EDIT_QUERY_WRAPPER_IMPLEMENTATION (ValueQuery)

EditValueQuery::Cursor EditValueQuery::Execute (Warehouse::ValueSequence _values) noexcept
{
    return Cursor {source.Execute (_values), eventsOnRemove, eventsOnChange, changeTracker};
}
} // namespace Emergence::Celerity

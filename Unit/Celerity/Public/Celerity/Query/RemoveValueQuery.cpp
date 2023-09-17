#include <Celerity/Query/RemoveValueQuery.hpp>

namespace Emergence::Celerity
{
EMERGENCE_CELERITY_REMOVE_QUERY_WRAPPER_IMPLEMENTATION (ValueQuery)

RemoveValueQuery::Cursor RemoveValueQuery::Execute (Warehouse::ValueSequence _values) noexcept
{
    return Cursor {source.Execute (_values), eventsOnRemove, eventsOnChange, changeTracker};
}
} // namespace Emergence::Celerity

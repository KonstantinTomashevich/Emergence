#include <Celerity/Query/ModifyValueQuery.hpp>

namespace Emergence::Celerity
{
EMERGENCE_CELERITY_MODIFY_QUERY_WRAPPER_IMPLEMENTATION (ValueQuery)

ModifyValueQuery::Cursor ModifyValueQuery::Execute (Warehouse::ValueSequence _values) noexcept
{
    return Cursor {source.Execute (_values), eventsOnRemove, eventsOnChange, changeTracker};
}
} // namespace Emergence::Celerity

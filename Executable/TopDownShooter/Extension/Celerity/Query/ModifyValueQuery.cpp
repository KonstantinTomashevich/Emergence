#include <Celerity/Query/ModifyValueQuery.hpp>

namespace Emergence::Celerity
{
WAREHOUSE_MODIFY_QUERY_WRAPPER_IMPLEMENTATION (ModifyValueQuery)

ModifyValueQuery::Cursor ModifyValueQuery::Execute (Warehouse::ValueSequence _values) noexcept
{
    return Cursor {source.Execute (_values), eventsOnRemove, changeTracker};
}
} // namespace Emergence::Celerity

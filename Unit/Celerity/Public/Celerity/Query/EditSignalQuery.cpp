#include <Celerity/Query/EditSignalQuery.hpp>

namespace Emergence::Celerity
{
EMERGENCE_CELERITY_EDIT_QUERY_WRAPPER_IMPLEMENTATION (SignalQuery)

EditSignalQuery::Cursor EditSignalQuery::Execute () noexcept
{
    return Cursor {source.Execute (), eventsOnRemove, eventsOnChange, changeTracker};
}
} // namespace Emergence::Celerity

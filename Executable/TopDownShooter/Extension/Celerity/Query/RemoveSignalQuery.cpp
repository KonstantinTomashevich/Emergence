#include <Celerity/Query/RemoveSignalQuery.hpp>

namespace Emergence::Celerity
{
EMERGENCE_CELERITY_REMOVE_QUERY_WRAPPER_IMPLEMENTATION (SignalQuery)

RemoveSignalQuery::Cursor RemoveSignalQuery::Execute () noexcept
{
    return Cursor {source.Execute (), eventsOnRemove, changeTracker};
}
} // namespace Emergence::Celerity

#include <Celerity/Query/ModifySignalQuery.hpp>

namespace Emergence::Celerity
{
EMERGENCE_CELERITY_MODIFY_QUERY_WRAPPER_IMPLEMENTATION (SignalQuery)

ModifySignalQuery::Cursor ModifySignalQuery::Execute () noexcept
{
    return Cursor {source.Execute (), eventsOnRemove, changeTracker};
}
} // namespace Emergence::Celerity

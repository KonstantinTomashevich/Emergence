#include <Celerity/Event/CascadeRemover.hpp>

namespace Emergence::Celerity
{
CascadeRemover::CascadeRemover (TaskConstructor &_constructor,
                                const StandardLayout::Mapping &_eventMapping,
                                const StandardLayout::Mapping &_objectMapping,
                                const Container::Vector<StandardLayout::FieldId> &_objectKeyFields) noexcept
    : TaskExecutorBase (_constructor),

      fetchEvents (_constructor.FetchSequence (_eventMapping)),
      removeObjects (_constructor.RemoveValue (_objectMapping, _objectKeyFields))
{
}

void CascadeRemover::Execute () noexcept
{
    for (auto eventCursor = fetchEvents.Execute (); const void *event = *eventCursor; ++eventCursor)
    {
        auto objectCursor = removeObjects.Execute (event);
        while (objectCursor.ReadConst ())
        {
            ~objectCursor;
        }
    }
}
} // namespace Emergence::Celerity

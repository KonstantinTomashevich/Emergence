#pragma once

#include <Celerity/PipelineBuilder.hpp>

namespace Emergence::Celerity
{
/// \brief Task that automatically removes objects, associated with events, after events are fired.
///
/// \details There are lots of cases when objects should be removed after some event, for example after unit death
///          event or after transform component removal event. This syntax-sugar-like task does removal automatically.
///
/// \warning Event pointer is used as object query argument! It is suitable for object-id-only events, but might not
///          be suitable for other cases.
///
/// \details It is recommended to use AS_CASCADE_REMOVER_1F macro for code readability. For example:
///          ```
///          _pipelineBuilder.AddTask ("Movement::RemoveAfterDeath"_us)
//               .AS_CASCADE_REMOVER_1F (DeathFixedEvent, MovementComponent, objectId)
//               .DependOn (Checkpoint::ASSEMBLY_FINISHED)
//               .DependOn (Checkpoint::MOVEMENT_STARTED);
///          ```
class CascadeRemover final : public TaskExecutorBase<CascadeRemover>
{
public:
    CascadeRemover (TaskConstructor &_constructor,
                    const StandardLayout::Mapping &_eventMapping,
                    const StandardLayout::Mapping &_objectMapping,
                    const Container::Vector<StandardLayout::FieldId> &_objectKeyFields) noexcept;

    void Execute () noexcept;

private:
    FetchSequenceQuery fetchEvents;
    RemoveValueQuery removeObjects;
};
} // namespace Emergence::Celerity

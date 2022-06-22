#pragma once

#include <Celerity/PipelineBuilder.hpp>

namespace Emergence::Celerity
{
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

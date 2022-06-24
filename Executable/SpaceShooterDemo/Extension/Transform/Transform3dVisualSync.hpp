#pragma once

#include <Celerity/PipelineBuilder.hpp>

namespace Emergence::Transform::VisualSync
{
struct Checkpoint final
{
    Checkpoint () = delete;

    static const Emergence::Memory::UniqueString SYNC_FINISHED;
};

void AddToNormalUpdate (Emergence::Celerity::PipelineBuilder &_pipelineBuilder) noexcept;
} // namespace Emergence::Transform::VisualSync

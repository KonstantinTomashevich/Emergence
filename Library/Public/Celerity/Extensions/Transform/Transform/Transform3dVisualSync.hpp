#pragma once

#include <Celerity/PipelineBuilder.hpp>

namespace Emergence::Celerity::VisualTransformSync
{
struct Checkpoint final
{
    Checkpoint () = delete;

    static const Emergence::Memory::UniqueString SYNC_FINISHED;
};

void AddToNormalUpdate (PipelineBuilder &_pipelineBuilder) noexcept;
} // namespace Emergence::Celerity::TransformVisualSync

#pragma once

#include <Celerity/PipelineBuilder.hpp>

namespace LevelGeneration
{
struct Checkpoint final
{
    Checkpoint () = delete;

    static const Emergence::Memory::UniqueString STEP_FINISHED;
};

void AddToLoadingPipeline (Emergence::Celerity::PipelineBuilder &_pipelineBuilder) noexcept;
} // namespace LevelGeneration

#pragma once

#include <Celerity/PipelineBuilder.hpp>

namespace LoadingOrchestration
{
struct Checkpoint final
{
    Checkpoint () = delete;

    static const Emergence::Memory::UniqueString FINISHED;
};

void AddToLoadingPipeline (Emergence::Celerity::PipelineBuilder &_pipelineBuilder,
                           bool *_loadingFinishedOutput) noexcept;
} // namespace LoadingOrchestration

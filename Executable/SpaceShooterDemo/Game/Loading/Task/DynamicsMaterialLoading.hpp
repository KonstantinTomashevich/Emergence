#pragma once

#include <Celerity/PipelineBuilder.hpp>

namespace DynamicsMaterialLoading
{
struct Checkpoint final
{
    Checkpoint () = delete;

    static const Emergence::Memory::UniqueString FINISHED;
};

void AddToLoadingPipeline (Emergence::Celerity::PipelineBuilder &_pipelineBuilder) noexcept;
} // namespace DynamicsMaterialLoading

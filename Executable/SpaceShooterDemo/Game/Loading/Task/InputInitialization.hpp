#pragma once

#include <Celerity/PipelineBuilder.hpp>

namespace InputInitialization
{
struct Checkpoint final
{
    Checkpoint () = delete;

    static const Emergence::Memory::UniqueString INITIALIZED;
};

void AddToLoadingPipeline (Emergence::Celerity::PipelineBuilder &_pipelineBuilder) noexcept;
} // namespace InputInitialization

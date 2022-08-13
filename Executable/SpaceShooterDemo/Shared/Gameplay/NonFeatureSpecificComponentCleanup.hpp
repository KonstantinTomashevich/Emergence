#pragma once

#include <Celerity/PipelineBuilder.hpp>

namespace NonFeatureSpecificComponentCleanup
{
struct Checkpoint final
{
    Checkpoint () = delete;

    static const Emergence::Memory::UniqueString STARTED;
    static const Emergence::Memory::UniqueString FINISHED;
};

void AddToFixedUpdate (Emergence::Celerity::PipelineBuilder &_pipelineBuilder) noexcept;
} // namespace NonFeatureSpecificComponentCleanup

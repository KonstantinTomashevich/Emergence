#pragma once

#include <Celerity/PipelineBuilder.hpp>

namespace Emergence::Celerity::BoundsCalculation2d
{
struct Checkpoint final
{
    Checkpoint () = delete;

    static const Memory::UniqueString STARTED;

    static const Memory::UniqueString FINISHED;
};

void AddToNormalUpdate (PipelineBuilder &_pipelineBuilder) noexcept;
} // namespace Emergence::Celerity::BoundsCalculation2d

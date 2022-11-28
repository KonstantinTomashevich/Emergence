#pragma once

#include <Celerity/PipelineBuilder.hpp>

namespace Emergence::Celerity::WorldRendering2d
{
struct Checkpoint final
{
    Checkpoint () = delete;

    static const Memory::UniqueString STARTED;

    static const Memory::UniqueString FINISHED;
};

void AddToNormalUpdate (PipelineBuilder &_pipelineBuilder, const Math::AxisAlignedBox2d &_worldBounds) noexcept;
} // namespace Emergence::Celerity::WorldRendering2d

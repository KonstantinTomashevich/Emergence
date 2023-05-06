#pragma once

#include <Celerity/PipelineBuilder.hpp>

#include <Math/AxisAlignedBox2d.hpp>

namespace Emergence::Celerity::Batching2d
{
struct Checkpoint final
{
    Checkpoint () = delete;

    static const Memory::UniqueString STARTED;

    static const Memory::UniqueString FINISHED;
};

void AddToNormalUpdate (PipelineBuilder &_pipelineBuilder, const Math::AxisAlignedBox2d &_worldBounds) noexcept;
} // namespace Emergence::Celerity::Batching2d

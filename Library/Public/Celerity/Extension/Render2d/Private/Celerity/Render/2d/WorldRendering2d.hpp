#pragma once

#include <Celerity/PipelineBuilder.hpp>

namespace Emergence::Celerity::WorldRendering2d
{
void AddToNormalUpdate (PipelineBuilder &_pipelineBuilder, const Math::AxisAlignedBox2d &_worldBounds) noexcept;
} // namespace Emergence::Celerity::WorldRendering2d

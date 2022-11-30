#pragma once

#include <Celerity/PipelineBuilder.hpp>

#include <Math/AxisAlignedBox2d.hpp>

namespace Emergence::Celerity::Rendering2d
{
/// \brief Adds tasks that update rendering data and perform rendering.
/// \details There is no checkpoints, because tasks are inserted into RenderPipelineFoundation.
void AddToNormalUpdate (PipelineBuilder &_pipelineBuilder, const Math::AxisAlignedBox2d &_worldBounds) noexcept;
} // namespace Emergence::Celerity::Rendering2d

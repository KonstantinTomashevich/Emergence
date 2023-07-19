#pragma once

#include <CelerityRenderFoundationApi.hpp>

#include <Celerity/PipelineBuilder.hpp>

namespace Emergence::Celerity::PostProcess
{
/// \brief Adds tasks that implement render command submission for PostProcessRenderPass instances.
/// \details There is no checkpoints, because tasks are inserted into RenderPipelineFoundation.
CelerityRenderFoundationApi void AddToNormalUpdate (PipelineBuilder &_pipelineBuilder) noexcept;
} // namespace Emergence::Celerity::PostProcess

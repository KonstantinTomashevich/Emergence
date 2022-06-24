#pragma once

#include <Celerity/PipelineBuilder.hpp>

namespace Assembly
{
void AddToFixedUpdate (Emergence::Celerity::PipelineBuilder &_pipelineBuilder) noexcept;

void AddToNormalUpdate (Emergence::Celerity::PipelineBuilder &_pipelineBuilder) noexcept;
} // namespace Assembly

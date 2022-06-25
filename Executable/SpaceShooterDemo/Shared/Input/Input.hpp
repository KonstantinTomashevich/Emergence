#pragma once

#include <Celerity/PipelineBuilder.hpp>

#include <Input/InputAccumulator.hpp>

namespace Input
{
void AddToFixedUpdate (Emergence::Celerity::PipelineBuilder &_pipelineBuilder) noexcept;

void AddToNormalUpdate (InputAccumulator *_inputAccumulator,
                        Emergence::Celerity::PipelineBuilder &_pipelineBuilder) noexcept;
} // namespace Input

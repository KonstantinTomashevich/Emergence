#pragma once

#include <SyntaxSugar/MuteWarnings.hpp>

#include <Celerity/InputAccumulator.hpp>
#include <Celerity/PipelineBuilder.hpp>

namespace Input
{
void AddToFixedUpdate (Emergence::Celerity::PipelineBuilder &_pipelineBuilder) noexcept;

void AddToNormalUpdate (Emergence::Celerity::InputAccumulator *_inputAccumulator,
                        Emergence::Celerity::PipelineBuilder &_pipelineBuilder) noexcept;
} // namespace Input

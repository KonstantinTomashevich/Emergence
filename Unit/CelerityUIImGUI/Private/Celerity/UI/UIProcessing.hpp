#pragma once

#include <Celerity/Input/FrameInputAccumulator.hpp>
#include <Celerity/PipelineBuilder.hpp>

namespace Emergence::Celerity::UIProcessing
{
void AddToNormalUpdate (PipelineBuilder &_pipelineBuilder,
                        FrameInputAccumulator *_inputAccumulator,
                        const KeyCodeMapping &_keyCodeMapping) noexcept;
} // namespace Emergence::Celerity::UIProcessing

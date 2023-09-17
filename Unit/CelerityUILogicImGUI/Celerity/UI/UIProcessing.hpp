#pragma once

#include <Celerity/PipelineBuilder.hpp>

#include <InputStorage/FrameInputAccumulator.hpp>

namespace Emergence::Celerity::UIProcessing
{
void AddToNormalUpdate (PipelineBuilder &_pipelineBuilder,
                        InputStorage::FrameInputAccumulator *_inputAccumulator,
                        const InputStorage::KeyCodeMapping &_keyCodeMapping) noexcept;
} // namespace Emergence::Celerity::UIProcessing

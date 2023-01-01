#pragma once

#include <Celerity/Input/FrameInputAccumulator.hpp>
#include <Celerity/PipelineBuilder.hpp>

namespace Emergence::Celerity::UIProcessing
{
struct Checkpoint final
{
    Checkpoint () = delete;

    static const Memory::UniqueString STARTED;

    static const Memory::UniqueString FINISHED;
};

void AddToNormalUpdate (PipelineBuilder &_pipelineBuilder,
                        FrameInputAccumulator *_inputAccumulator,
                        const KeyCodeMapping &_keyCodeMapping) noexcept;
} // namespace Emergence::Celerity::UIProcessing

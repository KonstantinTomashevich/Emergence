#pragma once

#include <Celerity/Input/FrameInputAccumulator.hpp>
#include <Celerity/PipelineBuilder.hpp>

namespace Emergence::Celerity::Input
{
struct Checkpoint final
{
    Checkpoint () = delete;

    static const Memory::UniqueString ACTION_DISPATCH_STARTED;

    static const Memory::UniqueString CUSTOM_ACTION_COMPONENT_INSERT_ALLOWED;

    static const Memory::UniqueString ACTION_COMPONENT_READ_ALLOWED;
};

void AddToFixedUpdate (PipelineBuilder &_builder) noexcept;

void AddToNormalUpdate (PipelineBuilder &_builder, FrameInputAccumulator *_inputAccumulator) noexcept;
} // namespace Emergence::Celerity::Input

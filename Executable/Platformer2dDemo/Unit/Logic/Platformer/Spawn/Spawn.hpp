#pragma once

#include <Platformer2dDemoLogicApi.hpp>

#include <Celerity/PipelineBuilder.hpp>

namespace Spawn
{
struct Platformer2dDemoLogicApi Checkpoint final
{
    Checkpoint () = delete;

    static const Emergence::Memory::UniqueString STARTED;
    static const Emergence::Memory::UniqueString FINISHED;
};

Platformer2dDemoLogicApi void AddToFixedUpdate (Emergence::Celerity::PipelineBuilder &_pipelineBuilder) noexcept;
} // namespace Spawn

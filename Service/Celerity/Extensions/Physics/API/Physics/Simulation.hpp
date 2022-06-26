#pragma once

#include <Celerity/PipelineBuilder.hpp>

namespace Emergence::Celerity::Simulation
{
struct Checkpoint final
{
    Checkpoint () = delete;

    static const Emergence::Memory::UniqueString SIMULATION_STARTED;
    static const Emergence::Memory::UniqueString SIMULATION_FINISHED;
};

void AddToFixedUpdate (PipelineBuilder &_pipelineBuilder) noexcept;
} // namespace Emergence::Celerity::Simulation

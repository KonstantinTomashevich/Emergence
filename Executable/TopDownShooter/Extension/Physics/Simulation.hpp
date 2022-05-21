#pragma once

#include <Celerity/PipelineBuilder.hpp>

namespace Emergence::Physics::Simulation
{
struct Checkpoint final
{
    Checkpoint () = delete;

    static const Emergence::Memory::UniqueString SIMULATION_STARTED;
    static const Emergence::Memory::UniqueString SIMULATION_FINISHED;
};

void AddToFixedUpdate (Emergence::Celerity::PipelineBuilder &_pipelineBuilder) noexcept;
} // namespace Emergence::Physics::Simulation

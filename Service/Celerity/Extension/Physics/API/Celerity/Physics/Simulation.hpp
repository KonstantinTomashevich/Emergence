#pragma once

#include <Celerity/PipelineBuilder.hpp>

namespace Emergence::Celerity::Simulation
{
/// \brief Contains checkpoints, supported by tasks from ::AddToFixedUpdate.
struct Checkpoint final
{
    Checkpoint () = delete;

    /// \brief Physics simulation execution starts after this checkpoint.
    static const Emergence::Memory::UniqueString SIMULATION_STARTED;

    /// \brief Physics simulation execution finishes before this checkpoint.
    static const Emergence::Memory::UniqueString SIMULATION_FINISHED;
};

/// \brief Adds tasks that execute world physics simulation.
void AddToFixedUpdate (PipelineBuilder &_pipelineBuilder) noexcept;
} // namespace Emergence::Celerity::Simulation

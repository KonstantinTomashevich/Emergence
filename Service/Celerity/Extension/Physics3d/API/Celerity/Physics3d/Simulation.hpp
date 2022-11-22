#pragma once

#include <Celerity/PipelineBuilder.hpp>

namespace Emergence::Celerity::Physics3dSimulation
{
/// \brief Contains checkpoints, supported by tasks from ::AddToFixedUpdate.
struct Checkpoint final
{
    Checkpoint () = delete;

    /// \brief Physics simulation execution starts after this checkpoint.
    static const Emergence::Memory::UniqueString STARTED;

    /// \brief Physics simulation execution finishes before this checkpoint.
    static const Emergence::Memory::UniqueString FINISHED;
};

/// \brief Adds tasks that execute world physics simulation.
void AddToFixedUpdate (PipelineBuilder &_pipelineBuilder) noexcept;
} // namespace Emergence::Celerity::Physics3dSimulation

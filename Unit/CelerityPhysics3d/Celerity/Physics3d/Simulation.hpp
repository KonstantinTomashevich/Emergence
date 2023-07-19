#pragma once

#include <CelerityPhysics3dApi.hpp>

#include <Celerity/PipelineBuilder.hpp>

namespace Emergence::Celerity::Physics3dSimulation
{
/// \brief Contains checkpoints, supported by tasks from ::AddToFixedUpdate.
struct CelerityPhysics3dApi Checkpoint final
{
    Checkpoint () = delete;

    /// \brief Physics simulation execution starts after this checkpoint.
    static const Emergence::Memory::UniqueString STARTED;

    /// \brief Physics simulation execution finishes before this checkpoint.
    static const Emergence::Memory::UniqueString FINISHED;
};

/// \brief Adds tasks that execute world physics simulation.
CelerityPhysics3dApi void AddToFixedUpdate (PipelineBuilder &_pipelineBuilder) noexcept;
} // namespace Emergence::Celerity::Physics3dSimulation

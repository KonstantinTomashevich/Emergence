#pragma once

#include <CelerityPhysics2dLogicApi.hpp>

#include <Celerity/PipelineBuilder.hpp>

namespace Emergence::Celerity::Physics2dSimulation
{
/// \brief Contains checkpoints, supported by tasks from ::AddToFixedUpdate.
struct CelerityPhysics2dLogicApi Checkpoint final
{
    Checkpoint () = delete;

    /// \brief Physics simulation execution starts after this checkpoint.
    static const Emergence::Memory::UniqueString STARTED;

    /// \brief Physics simulation execution finishes before this checkpoint.
    static const Emergence::Memory::UniqueString FINISHED;
};

/// \brief Adds tasks that execute world physics simulation.
CelerityPhysics2dLogicApi void AddToFixedUpdate (PipelineBuilder &_pipelineBuilder) noexcept;
} // namespace Emergence::Celerity::Physics2dSimulation

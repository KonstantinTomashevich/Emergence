#include <Physics/Simulation.hpp>

#include <Shared/CelerityUtils.hpp>
#include <Shared/Checkpoint.hpp>

#include <Transform/Transform3dVisualSync.hpp>

namespace Emergence::Celerity
{
void AddAllCheckpoints (PipelineBuilder &_pipelineBuilder) noexcept
{
    _pipelineBuilder.AddCheckpoint (Checkpoint::ASSEMBLY_STARTED);
    _pipelineBuilder.AddCheckpoint (Checkpoint::ASSEMBLY_FINISHED);

    _pipelineBuilder.AddCheckpoint (Checkpoint::INPUT_DISPATCH_STARTED);
    _pipelineBuilder.AddCheckpoint (Checkpoint::INPUT_LISTENERS_PUSH_ALLOWED);
    _pipelineBuilder.AddCheckpoint (Checkpoint::INPUT_LISTENERS_READ_ALLOWED);

    _pipelineBuilder.AddCheckpoint (Checkpoint::MORTALITY_STARTED);
    _pipelineBuilder.AddCheckpoint (Checkpoint::MORTALITY_FINISHED);

    _pipelineBuilder.AddCheckpoint (Checkpoint::RENDER_UPDATE_STARTED);
    _pipelineBuilder.AddCheckpoint (Checkpoint::RENDER_UPDATE_FINISHED);

    _pipelineBuilder.AddCheckpoint (Physics::Simulation::Checkpoint::SIMULATION_STARTED);
    _pipelineBuilder.AddCheckpoint (Physics::Simulation::Checkpoint::SIMULATION_FINISHED);
    _pipelineBuilder.AddCheckpoint (Transform::VisualSync::Checkpoint::SYNC_FINISHED);
}
} // namespace Emergence::Celerity

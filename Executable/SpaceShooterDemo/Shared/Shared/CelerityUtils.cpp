#include <Celerity/Physics/Simulation.hpp>
#include <Celerity/Transform/Transform3dVisualSync.hpp>

#include <Shared/CelerityUtils.hpp>
#include <Shared/Checkpoint.hpp>

namespace Emergence::Celerity
{
void AddAllCheckpoints (PipelineBuilder &_pipelineBuilder) noexcept
{
    _pipelineBuilder.AddCheckpoint (Checkpoint::ASSEMBLY_STARTED);
    _pipelineBuilder.AddCheckpoint (Checkpoint::ASSEMBLY_FINISHED);

    _pipelineBuilder.AddCheckpoint (Checkpoint::CAMERA_UPDATE_STARTED);
    _pipelineBuilder.AddCheckpoint (Checkpoint::CAMERA_UPDATE_FINISHED);

    _pipelineBuilder.AddCheckpoint (Checkpoint::DAMAGE_STARTED);
    _pipelineBuilder.AddCheckpoint (Checkpoint::DAMAGE_FINISHED);

    _pipelineBuilder.AddCheckpoint (Checkpoint::INPUT_DISPATCH_STARTED);
    _pipelineBuilder.AddCheckpoint (Checkpoint::INPUT_LISTENERS_PUSH_ALLOWED);
    _pipelineBuilder.AddCheckpoint (Checkpoint::INPUT_LISTENERS_READ_ALLOWED);

    _pipelineBuilder.AddCheckpoint (Checkpoint::MORTALITY_STARTED);
    _pipelineBuilder.AddCheckpoint (Checkpoint::MORTALITY_FINISHED);

    _pipelineBuilder.AddCheckpoint (Checkpoint::MOVEMENT_STARTED);
    _pipelineBuilder.AddCheckpoint (Checkpoint::MOVEMENT_FINISHED);

    _pipelineBuilder.AddCheckpoint (Checkpoint::RENDER_UPDATE_STARTED);
    _pipelineBuilder.AddCheckpoint (Checkpoint::RENDER_UPDATE_FINISHED);

    _pipelineBuilder.AddCheckpoint (Checkpoint::SHOOTING_STARTED);
    _pipelineBuilder.AddCheckpoint (Checkpoint::SHOOTING_FINISHED);

    _pipelineBuilder.AddCheckpoint (Checkpoint::SPAWN_STARTED);
    _pipelineBuilder.AddCheckpoint (Checkpoint::SPAWN_FINISHED);

    _pipelineBuilder.AddCheckpoint (Simulation::Checkpoint::SIMULATION_STARTED);
    _pipelineBuilder.AddCheckpoint (Simulation::Checkpoint::SIMULATION_FINISHED);
    _pipelineBuilder.AddCheckpoint (VisualTransformSync::Checkpoint::SYNC_FINISHED);
}
} // namespace Emergence::Celerity

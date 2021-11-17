#include <Shared/CelerityUtils.hpp>
#include <Shared/Checkpoint.hpp>

namespace Emergence::Celerity
{
void AddAllCheckpoints (PipelineBuilder &_pipelineBuilder) noexcept
{
    _pipelineBuilder.AddCheckpoint (Checkpoint::TIME_UPDATED);
    _pipelineBuilder.AddCheckpoint (Checkpoint::INPUT_ROUTING_FINISHED);
}
} // namespace Emergence::Celerity

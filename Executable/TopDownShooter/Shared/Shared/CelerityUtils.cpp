#include <Shared/CelerityUtils.hpp>
#include <Shared/Checkpoint.hpp>

namespace Emergence::Celerity
{
void AddAllCheckpoints (PipelineBuilder &_pipelineBuilder) noexcept
{
    _pipelineBuilder.AddCheckpoint (Checkpoint::TIME_UPDATED);
    _pipelineBuilder.AddCheckpoint (Checkpoint::INPUT_DISPATCH_BEGIN);
    _pipelineBuilder.AddCheckpoint (Checkpoint::INPUT_DISPATCH_END);
    _pipelineBuilder.AddCheckpoint (Checkpoint::FRAME_STATS_COLLECTION);
}
} // namespace Emergence::Celerity

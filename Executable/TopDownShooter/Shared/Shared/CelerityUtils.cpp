#include <Shared/CelerityUtils.hpp>
#include <Shared/Checkpoint.hpp>

namespace Emergence::Celerity
{
void AddAllCheckpoints (PipelineBuilder &_pipelineBuilder) noexcept
{
    _pipelineBuilder.AddCheckpoint (Checkpoint::INPUT_DISPATCH_STARTED);
    _pipelineBuilder.AddCheckpoint (Checkpoint::INPUT_LISTENERS_PUSH_ALLOWED);
    _pipelineBuilder.AddCheckpoint (Checkpoint::INPUT_LISTENERS_READ_ALLOWED);
}
} // namespace Emergence::Celerity

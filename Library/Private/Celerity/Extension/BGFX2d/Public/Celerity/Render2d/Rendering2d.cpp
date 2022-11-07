#include <Celerity/Render2d/BoundsCalculation2d.hpp>
#include <Celerity/Render2d/Rendering2d.hpp>
#include <Celerity/Render2d/WorldRendering2d.hpp>

namespace Emergence::Celerity::Rendering2d
{
const Memory::UniqueString Checkpoint::STARTED {"Rendering2dStarted"};
const Memory::UniqueString Checkpoint::FINISHED {"Rendering2dFinished"};

void AddToNormalUpdate (PipelineBuilder &_pipelineBuilder, const Math::AxisAlignedBox2d &_worldBounds) noexcept
{
    _pipelineBuilder.AddCheckpoint (Checkpoint::STARTED);
    _pipelineBuilder.AddCheckpoint (Checkpoint::FINISHED);
    BoundsCalculation2d::AddToNormalUpdate (_pipelineBuilder);
    WorldRendering2d::AddToNormalUpdate (_pipelineBuilder, _worldBounds);
}
} // namespace Emergence::Celerity::Rendering2d

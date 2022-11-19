#include <Celerity/PipelineBuilderMacros.hpp>
#include <Celerity/Render2d/BoundsCalculation2d.hpp>
#include <Celerity/Render2d/Camera2dComponent.hpp>
#include <Celerity/Render2d/Rendering2d.hpp>
#include <Celerity/Render2d/Sprite2dComponent.hpp>
#include <Celerity/Render2d/WorldRendering2d.hpp>
#include <Celerity/Transform/Events.hpp>
#include <Celerity/Transform/TransformHierarchyCleanup.hpp>

namespace Emergence::Celerity::Rendering2d
{
const Memory::UniqueString Checkpoint::STARTED {"Rendering2dStarted"};
const Memory::UniqueString Checkpoint::FINISHED {"Rendering2dFinished"};

void AddToNormalUpdate (PipelineBuilder &_pipelineBuilder, const Math::AxisAlignedBox2d &_worldBounds) noexcept
{
    using namespace Memory::Literals;

    _pipelineBuilder.AddCheckpoint (Checkpoint::STARTED);
    _pipelineBuilder.AddCheckpoint (Checkpoint::FINISHED);

    _pipelineBuilder.AddTask ("CleanupCamera2dComponentAfterTransformRemovalFromNormal"_us)
        .AS_CASCADE_REMOVER_1F (Emergence::Celerity::Transform2dComponentRemovedNormalEvent, Camera2dComponent,
                                objectId)
        .DependOn (Checkpoint::STARTED)
        .DependOn (TransformHierarchyCleanup::Checkpoint::DETACHED_REMOVAL_FINISHED)
        .MakeDependencyOf ("CleanupCamera2dComponentAfterTransformRemovalFromFixed"_us);

    _pipelineBuilder.AddTask ("CleanupCamera2dComponentAfterTransformRemovalFromFixed"_us)
        .AS_CASCADE_REMOVER_1F (Emergence::Celerity::Transform2dComponentRemovedFixedToNormalEvent, Camera2dComponent,
                                objectId)
        .MakeDependencyOf (BoundsCalculation2d::Checkpoint::STARTED);

    _pipelineBuilder.AddTask ("CleanupSprite2dComponentAfterTransformRemovalFromNormal"_us)
        .AS_CASCADE_REMOVER_1F (Emergence::Celerity::Transform2dComponentRemovedNormalEvent, Sprite2dComponent,
                                objectId)
        .DependOn (Checkpoint::STARTED)
        .DependOn (TransformHierarchyCleanup::Checkpoint::DETACHED_REMOVAL_FINISHED)
        .MakeDependencyOf ("CleanupSprite2dComponentAfterTransformRemovalFromFixed"_us);

    _pipelineBuilder.AddTask ("CleanupSprite2dComponentAfterTransformRemovalFromFixed"_us)
        .AS_CASCADE_REMOVER_1F (Emergence::Celerity::Transform2dComponentRemovedFixedToNormalEvent, Sprite2dComponent,
                                objectId)
        .MakeDependencyOf (BoundsCalculation2d::Checkpoint::STARTED);

    BoundsCalculation2d::AddToNormalUpdate (_pipelineBuilder);
    WorldRendering2d::AddToNormalUpdate (_pipelineBuilder, _worldBounds);
}
} // namespace Emergence::Celerity::Rendering2d

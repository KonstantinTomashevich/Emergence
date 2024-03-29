#include <Celerity/Asset/AssetManagement.hpp>
#include <Celerity/PipelineBuilderMacros.hpp>
#include <Celerity/Render/2d/Batching2d.hpp>
#include <Celerity/Render/2d/BoundsCalculation2d.hpp>
#include <Celerity/Render/2d/Camera2dComponent.hpp>
#include <Celerity/Render/2d/DebugShape2dComponent.hpp>
#include <Celerity/Render/2d/Rendering2d.hpp>
#include <Celerity/Render/2d/Sprite2dComponent.hpp>
#include <Celerity/Render/2d/Sprite2dUvAnimationComponent.hpp>
#include <Celerity/Render/2d/Sprite2dUvAnimationSync.hpp>
#include <Celerity/Render/2d/WorldRendering2d.hpp>
#include <Celerity/Transform/Events.hpp>
#include <Celerity/Transform/TransformHierarchyCleanup.hpp>

namespace Emergence::Celerity::Rendering2d
{
void AddToNormalUpdate (PipelineBuilder &_pipelineBuilder, const Math::AxisAlignedBox2d &_worldBounds) noexcept
{
    using namespace Memory::Literals;

    _pipelineBuilder.AddTask ("RemoveCamera2dComponentOnTransformCleanup"_us)
        .AS_CASCADE_REMOVER_1F (TransformNodeCleanupNormalEvent, Camera2dComponent, objectId)
        .DependOn (TransformHierarchyCleanup::Checkpoint::CLEANUP_STARTED)
        .MakeDependencyOf (TransformHierarchyCleanup::Checkpoint::FINISHED);

    _pipelineBuilder.AddTask ("RemoveSprite2dComponentOnTransformCleanup"_us)
        .AS_CASCADE_REMOVER_1F (TransformNodeCleanupNormalEvent, Sprite2dComponent, objectId)
        .DependOn (TransformHierarchyCleanup::Checkpoint::CLEANUP_STARTED)
        .MakeDependencyOf (TransformHierarchyCleanup::Checkpoint::FINISHED)
        // We delay removal processing in asset management by one frame, because otherwise
        // we won't be able to delete Sprite2dComponent after AssetManagement.
        .DependOn (AssetManagement::Checkpoint::FINISHED);

    _pipelineBuilder.AddTask ("RemoveSprite2dUvAnimationComponentOnTransformCleanup"_us)
        .AS_CASCADE_REMOVER_1F (TransformNodeCleanupNormalEvent, Sprite2dUvAnimationComponent, objectId)
        .DependOn (TransformHierarchyCleanup::Checkpoint::CLEANUP_STARTED)
        .MakeDependencyOf (TransformHierarchyCleanup::Checkpoint::FINISHED)
        // We delay removal processing in asset management by one frame, because otherwise
        // we won't be able to delete DebugShape2dComponent after AssetManagement.
        .DependOn (AssetManagement::Checkpoint::FINISHED);

    _pipelineBuilder.AddTask ("RemoveDebugShape2dComponentOnTransformCleanup"_us)
        .AS_CASCADE_REMOVER_1F (TransformNodeCleanupNormalEvent, DebugShape2dComponent, objectId)
        .DependOn (TransformHierarchyCleanup::Checkpoint::CLEANUP_STARTED)
        .MakeDependencyOf (TransformHierarchyCleanup::Checkpoint::FINISHED)
        // We delay removal processing in asset management by one frame, because otherwise
        // we won't be able to delete DebugShape2dComponent after AssetManagement.
        .DependOn (AssetManagement::Checkpoint::FINISHED);

    auto visualGroup = _pipelineBuilder.OpenVisualGroup ("Rendering2d");
    Batching2d::AddToNormalUpdate (_pipelineBuilder, _worldBounds);
    BoundsCalculation2d::AddToNormalUpdate (_pipelineBuilder);
    Sprite2dUvAnimationSync::AddToNormalUpdate (_pipelineBuilder);
    WorldRendering2d::AddToNormalUpdate (_pipelineBuilder);
}
} // namespace Emergence::Celerity::Rendering2d

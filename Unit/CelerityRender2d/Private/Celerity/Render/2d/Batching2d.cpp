#include <limits>

#include <Celerity/Asset/Asset.hpp>
#include <Celerity/PipelineBuilderMacros.hpp>
#include <Celerity/Render/2d/Batching2d.hpp>
#include <Celerity/Render/2d/Batching2dSingleton.hpp>
#include <Celerity/Render/2d/BoundsCalculation2d.hpp>
#include <Celerity/Render/2d/Camera2dComponent.hpp>
#include <Celerity/Render/2d/DebugShape2dComponent.hpp>
#include <Celerity/Render/2d/RenderObject2dComponent.hpp>
#include <Celerity/Render/2d/Sprite2dComponent.hpp>
#include <Celerity/Render/2d/World2dRenderPass.hpp>
#include <Celerity/Render/2d/WorldRendering2d.hpp>
#include <Celerity/Render/Foundation/Events.hpp>
#include <Celerity/Render/Foundation/RenderPipelineFoundation.hpp>
#include <Celerity/Render/Foundation/Viewport.hpp>
#include <Celerity/Transform/TransformComponent.hpp>
#include <Celerity/Transform/TransformWorldAccessor.hpp>

#include <Render/Backend/Renderer.hpp>

namespace Emergence::Celerity::Batching2d
{
const Memory::UniqueString Checkpoint::STARTED {"Batching2d::Started"};
const Memory::UniqueString Checkpoint::FINISHED {"Batching2d::Finished"};

class Batching2dExecutor final : public TaskExecutorBase<Batching2dExecutor>
{
public:
    Batching2dExecutor (TaskConstructor &_constructor, const Math::AxisAlignedBox2d &_worldBounds) noexcept;

    void Execute () noexcept;

private:
    ModifySingletonQuery modifyBatching;
    FetchAscendingRangeQuery fetchRenderPassesByNameAscending;
    FetchValueQuery fetchViewportByName;
    FetchValueQuery fetchCameraById;

    FetchValueQuery fetchTransformById;
    Transform2dWorldAccessor transformWorldAccessor;

    FetchShapeIntersectionQuery fetchVisibleRenderObjects;
    FetchValueQuery fetchLocalBoundsByRenderObjectId;
    FetchValueQuery fetchSpriteByObjectId;
    FetchValueQuery fetchDebugShapeByObjectId;
};

static Container::Vector<Warehouse::Dimension> GetDimensions (const Math::AxisAlignedBox2d &_worldBounds)
{
    const StandardLayout::FieldId minField = StandardLayout::ProjectNestedField (
        RenderObject2dComponent::Reflect ().global, Math::AxisAlignedBox2d::Reflect ().min);
    const StandardLayout::FieldId minXField =
        StandardLayout::ProjectNestedField (minField, Math::Vector2f::Reflect ().x);
    const StandardLayout::FieldId minYField =
        StandardLayout::ProjectNestedField (minField, Math::Vector2f::Reflect ().y);

    const StandardLayout::FieldId maxField = StandardLayout::ProjectNestedField (
        RenderObject2dComponent::Reflect ().global, Math::AxisAlignedBox2d::Reflect ().max);
    const StandardLayout::FieldId maxXField =
        StandardLayout::ProjectNestedField (maxField, Math::Vector2f::Reflect ().x);
    const StandardLayout::FieldId maxYField =
        StandardLayout::ProjectNestedField (maxField, Math::Vector2f::Reflect ().y);

    return {
        {
            &_worldBounds.min.x,
            RenderObject2dComponent::Reflect ().mapping.GetField (minXField),
            &_worldBounds.max.x,
            RenderObject2dComponent::Reflect ().mapping.GetField (maxXField),
        },
        {
            &_worldBounds.min.y,
            RenderObject2dComponent::Reflect ().mapping.GetField (minYField),
            &_worldBounds.max.y,
            RenderObject2dComponent::Reflect ().mapping.GetField (maxYField),
        },
    };
}

Batching2dExecutor::Batching2dExecutor (TaskConstructor &_constructor,
                                        const Math::AxisAlignedBox2d &_worldBounds) noexcept
    : TaskExecutorBase (_constructor),

      modifyBatching (MODIFY_SINGLETON (Batching2dSingleton)),
      fetchRenderPassesByNameAscending (FETCH_ASCENDING_RANGE (World2dRenderPass, name)),
      fetchViewportByName (FETCH_VALUE_1F (Viewport, name)),
      fetchCameraById (FETCH_VALUE_1F (Camera2dComponent, objectId)),

      fetchTransformById (FETCH_VALUE_1F (Transform2dComponent, objectId)),
      transformWorldAccessor (_constructor),

      fetchVisibleRenderObjects (_constructor.FetchShapeIntersection (RenderObject2dComponent::Reflect ().mapping,
                                                                      GetDimensions (_worldBounds))),
      fetchLocalBoundsByRenderObjectId (FETCH_VALUE_1F (LocalBounds2dComponent, renderObjectId)),
      fetchSpriteByObjectId (FETCH_VALUE_1F (Sprite2dComponent, objectId)),
      fetchDebugShapeByObjectId (FETCH_VALUE_1F (DebugShape2dComponent, objectId))
{
    _constructor.DependOn (BoundsCalculation2d::Checkpoint::FINISHED);
    _constructor.DependOn (RenderPipelineFoundation::Checkpoint::VIEWPORT_SYNC_FINISHED);
    _constructor.DependOn (Checkpoint::STARTED);
    _constructor.MakeDependencyOf (Checkpoint::FINISHED);
    _constructor.MakeDependencyOf (WorldRendering2d::Checkpoint::STARTED);
}

void Batching2dExecutor::Execute () noexcept
{
    auto batchingCursor = modifyBatching.Execute ();
    auto *batching = static_cast<Batching2dSingleton *> (*batchingCursor);

    for (auto passCursor = fetchRenderPassesByNameAscending.Execute (nullptr, nullptr);
         const auto *pass = static_cast<const World2dRenderPass *> (*passCursor); ++passCursor)
    {
        auto viewportCursor = fetchViewportByName.Execute (&pass->name);
        const auto *viewport = static_cast<const Viewport *> (*viewportCursor);

        if (!viewport)
        {
            continue;
        }

        auto cameraCursor = fetchCameraById.Execute (&pass->cameraObjectId);
        const auto *camera = static_cast<const Camera2dComponent *> (*cameraCursor);

        if (!camera)
        {
            continue;
        }

        auto cameraTransformCursor = fetchTransformById.Execute (&pass->cameraObjectId);
        const auto *cameraTransform = static_cast<const Transform2dComponent *> (*cameraTransformCursor);

        if (!cameraTransform)
        {
            continue;
        }

        auto &viewportInfo = batching->viewports.emplace_back (
            ViewportInfoContainer {viewport->name,
                                   {},
                                   Math::Vector2f::ZERO,
                                   Container::Vector<Batch2d> {batching->viewports.get_allocator ()}});

        viewportInfo.cameraTransform = cameraTransform->GetVisualWorldTransform (transformWorldAccessor);
        const float aspectRatio = static_cast<float> (viewport->width) / static_cast<float> (viewport->height);
        viewportInfo.cameraHalfOrthographicSize = {camera->halfOrthographicSize * aspectRatio,
                                                   camera->halfOrthographicSize};

        const Math::AxisAlignedBox2d localVisibilityBox {-viewportInfo.cameraHalfOrthographicSize,
                                                         viewportInfo.cameraHalfOrthographicSize};
        const Math::Matrix3x3f cameraTransformMatrix {viewportInfo.cameraTransform};
        const Math::AxisAlignedBox2d globalVisibilityBox = cameraTransformMatrix * localVisibilityBox;

        struct
        {
            float minX;
            float maxX;
            float minY;
            float maxY;
        } query;

        query.minX = globalVisibilityBox.min.x;
        query.maxX = globalVisibilityBox.max.x;
        query.minY = globalVisibilityBox.min.y;
        query.maxY = globalVisibilityBox.max.y;

        for (auto renderObjectCursor = fetchVisibleRenderObjects.Execute (&query);
             const auto *renderObject = static_cast<const RenderObject2dComponent *> (*renderObjectCursor);
             ++renderObjectCursor)
        {
            // We do not check intersections with original rotated rect, because we're designing the algorithm
            // around the most popular use cases, so we do not optimize for rare case of camera rotation.

            for (auto localBoundsCursor = fetchLocalBoundsByRenderObjectId.Execute (&renderObject->objectId);
                 const auto *localBounds = static_cast<const LocalBounds2dComponent *> (*localBoundsCursor);
                 ++localBoundsCursor)
            {
                // We do not check local bounds intersection here as it is unneeded in most cases and may throw the
                // visual out only on rare occasion. Therefore, it is better for performance to avoid this check.

                for (auto spriteCursor = fetchSpriteByObjectId.Execute (&localBounds->objectId);
                     const auto *sprite = static_cast<const Sprite2dComponent *> (*spriteCursor); ++spriteCursor)
                {
                    if (sprite->visibilityMask & camera->visibilityMask)
                    {
                        batching->GetBatch (batching->viewports.size () - 1u, sprite->layer, sprite->materialInstanceId)
                            .sprites.emplace_back (sprite->spriteId);
                    }
                }

                for (auto debugShapeCursor = fetchDebugShapeByObjectId.Execute (&localBounds->objectId);
                     const auto *debugShape = static_cast<const DebugShape2dComponent *> (*debugShapeCursor);
                     ++debugShapeCursor)
                {
                    batching
                        ->GetBatch (batching->viewports.size () - 1u, std::numeric_limits<std::uint16_t>::max (),
                                    debugShape->materialInstanceId)
                        .debugShapes.emplace_back (debugShape->debugShapeId);
                }
            }
        }
    }
}

void AddToNormalUpdate (PipelineBuilder &_pipelineBuilder, const Math::AxisAlignedBox2d &_worldBounds) noexcept
{
    using namespace Memory::Literals;

    auto visualGroup = _pipelineBuilder.OpenVisualGroup ("Batching2d");
    _pipelineBuilder.AddTask ("ClearWorld2dRenderPassesAfterViewportRemoval"_us)
        .AS_CASCADE_REMOVER_1F (ViewportRemovedNormalEvent, World2dRenderPass, name)
        .DependOn (RenderPipelineFoundation::Checkpoint::RENDER_STARTED)
        .MakeDependencyOf ("Batching2dExecutor"_us);

    _pipelineBuilder.AddCheckpoint (Checkpoint::STARTED);
    _pipelineBuilder.AddCheckpoint (Checkpoint::FINISHED);
    _pipelineBuilder.AddTask ("Batching2dExecutor"_us).SetExecutor<Batching2dExecutor> (_worldBounds);
}
} // namespace Emergence::Celerity::Batching2d

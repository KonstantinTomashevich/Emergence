#include <Celerity/PipelineBuilderMacros.hpp>
#include <Celerity/Render2d/BGFX/Rendering.hpp>
#include <Celerity/Render2d/BackendApi.hpp>
#include <Celerity/Render2d/BoundsCalculation2d.hpp>
#include <Celerity/Render2d/Camera2dComponent.hpp>
#include <Celerity/Render2d/Render2dSingleton.hpp>
#include <Celerity/Render2d/RenderObject2dComponent.hpp>
#include <Celerity/Render2d/Rendering2d.hpp>
#include <Celerity/Render2d/Sprite2dComponent.hpp>
#include <Celerity/Render2d/WorldRendering2d.hpp>
#include <Celerity/Transform/TransformComponent.hpp>
#include <Celerity/Transform/TransformWorldAccessor.hpp>

namespace Emergence::Celerity::WorldRendering2d
{
const Memory::UniqueString Checkpoint::STARTED {"WorldRendering2dStarted"};
const Memory::UniqueString Checkpoint::FINISHED {"WorldRendering2dFinished"};

class WorldRenderer final : public TaskExecutorBase<WorldRenderer>
{
public:
    WorldRenderer (TaskConstructor &_constructor, const Math::AxisAlignedBox2d &_worldBounds) noexcept;

    void Execute () noexcept;

private:
    struct Batch
    {
        uint16_t layer = 0u;
        Memory::UniqueString materialInstanceId;
        Container::Vector<BGFX::RectData> rects;
    };

    void CollectVisibleObjects () noexcept;

    WorldRenderer::Batch &GetBatch (uint16_t _layer, Memory::UniqueString _materialInstanceId) noexcept;

    void SubmitBatches () noexcept;

    void PoolBatches () noexcept;

    FetchSingletonQuery fetchRenderSingleton;
    FetchValueQuery fetchCameraById;

    FetchValueQuery fetchTransformById;
    Transform2dWorldAccessor transformWorldAccessor;

    FetchShapeIntersectionQuery fetchVisibleRenderObjects;
    FetchValueQuery fetchLocalBoundsByRenderObjectId;
    FetchValueQuery fetchSpriteByObjectId;

    Math::Transform2d selectedCameraTransform;
    Math::Vector2f selectedCameraHalfOrthographicSize {Math::Vector2f::ZERO};

    BGFX::RenderingBackend renderingBackend;
    Container::Vector<Batch> batches {Memory::Profiler::AllocationGroup::Top ()};
    Container::Vector<Batch> batchPool {Memory::Profiler::AllocationGroup::Top ()};
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

WorldRenderer::WorldRenderer (TaskConstructor &_constructor, const Math::AxisAlignedBox2d &_worldBounds) noexcept
    : fetchRenderSingleton (FETCH_SINGLETON (Render2dSingleton)),
      fetchCameraById (FETCH_VALUE_1F (Camera2dComponent, objectId)),

      fetchTransformById (FETCH_VALUE_1F (Transform2dComponent, objectId)),
      transformWorldAccessor (_constructor),

      fetchVisibleRenderObjects (_constructor.FetchShapeIntersection (RenderObject2dComponent::Reflect ().mapping,
                                                                      GetDimensions (_worldBounds))),
      fetchLocalBoundsByRenderObjectId (FETCH_VALUE_1F (LocalBounds2dComponent, renderObjectId)),
      fetchSpriteByObjectId (FETCH_VALUE_1F (Sprite2dComponent, objectId)),

      renderingBackend (_constructor)
{
    _constructor.DependOn (Rendering2d::Checkpoint::STARTED);
    _constructor.DependOn (Checkpoint::STARTED);
    _constructor.DependOn (BoundsCalculation2d::Checkpoint::FINISHED);
    _constructor.MakeDependencyOf (Checkpoint::FINISHED);
    _constructor.MakeDependencyOf (Rendering2d::Checkpoint::FINISHED);
}

void WorldRenderer::Execute () noexcept
{
    CollectVisibleObjects ();
    SubmitBatches ();
    PoolBatches ();
}

void WorldRenderer::CollectVisibleObjects () noexcept
{
    selectedCameraTransform = {};
    selectedCameraHalfOrthographicSize = Math::Vector2f::ZERO;

    auto renderCursor = fetchRenderSingleton.Execute ();
    const auto *render = static_cast<const Render2dSingleton *> (*renderCursor);

    auto cameraCursor = fetchCameraById.Execute (&render->cameraObjectId);
    const auto *camera = static_cast<const Camera2dComponent *> (*cameraCursor);

    if (!camera)
    {
        return;
    }

    auto cameraTransformCursor = fetchTransformById.Execute (&render->cameraObjectId);
    const auto *cameraTransform = static_cast<const Transform2dComponent *> (*cameraTransformCursor);

    if (!cameraTransform)
    {
        return;
    }

    selectedCameraTransform = cameraTransform->GetVisualWorldTransform (transformWorldAccessor);
    const Render2dBackendConfig &backendConfig = Render2dBackend::GetCurrentConfig ();
    const float aspectRatio = static_cast<float> (backendConfig.width) / static_cast<float> (backendConfig.height);
    selectedCameraHalfOrthographicSize = {camera->halfOrthographicSize * aspectRatio, camera->halfOrthographicSize};

    const Math::AxisAlignedBox2d localVisibilityBox {-selectedCameraHalfOrthographicSize,
                                                     selectedCameraHalfOrthographicSize};
    const Math::Matrix3x3f cameraTransformMatrix {selectedCameraTransform};
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

            if (auto transformCursor = fetchTransformById.Execute (&localBounds->objectId);
                const auto *boundsTransform = static_cast<const Transform2dComponent *> (*transformCursor))
            {
                const Math::Transform2d worldTransform =
                    boundsTransform->GetVisualWorldTransform (transformWorldAccessor);

                for (auto spriteCursor = fetchSpriteByObjectId.Execute (&localBounds->objectId);
                     const auto *sprite = static_cast<const Sprite2dComponent *> (*spriteCursor); ++spriteCursor)
                {
                    GetBatch (sprite->layer, sprite->materialInstanceId)
                        .rects.emplace_back (BGFX::RectData {worldTransform, sprite->uv, sprite->halfSize});
                }
            }
        }
    }
}

WorldRenderer::Batch &WorldRenderer::GetBatch (uint16_t _layer, Memory::UniqueString _materialInstanceId) noexcept
{
    auto next = std::upper_bound (batches.begin (), batches.end (), std::make_pair (_layer, _materialInstanceId),
                                  [] (const auto &_query, const Batch &_batch)
                                  {
                                      if (_query.first != _batch.layer)
                                      {
                                          return _query.first < _batch.layer;
                                      }

                                      return *_query.second < *_batch.materialInstanceId;
                                  });

    if (next != batches.begin ())
    {
        auto previous = next - 1u;
        if (previous->layer == _layer && previous->materialInstanceId == _materialInstanceId)
        {
            return *previous;
        }
    }

    if (batchPool.empty ())
    {
        return *batches.emplace (
            next, Batch {_layer, _materialInstanceId, Container::Vector<BGFX::RectData> {batches.get_allocator ()}});
    }

    Batch &pooledBatch = batchPool.back ();
    batchPool.pop_back ();

    pooledBatch.layer = _layer;
    pooledBatch.materialInstanceId = _materialInstanceId;
    pooledBatch.rects.clear ();
    return *batches.emplace (next, std::move (pooledBatch));
}

void WorldRenderer::SubmitBatches () noexcept
{
    decltype (renderingBackend)::SubmitCamera (selectedCameraTransform, selectedCameraHalfOrthographicSize);
    for (const Batch &batch : batches)
    {
        renderingBackend.SubmitMaterialInstance (batch.materialInstanceId);
        renderingBackend.SubmitRects (batch.rects);
    }

    renderingBackend.EndFrame ();
}

void WorldRenderer::PoolBatches () noexcept
{
    for (Batch &batch : batches)
    {
        batchPool.emplace_back (std::move (batch));
    }

    batches.clear ();
}

void AddToNormalUpdate (PipelineBuilder &_pipelineBuilder, const Math::AxisAlignedBox2d &_worldBounds) noexcept
{
    _pipelineBuilder.AddCheckpoint (Checkpoint::STARTED);
    _pipelineBuilder.AddCheckpoint (Checkpoint::FINISHED);
    _pipelineBuilder.AddTask (Memory::UniqueString {"WorldRenderer2d"}).SetExecutor<WorldRenderer> (_worldBounds);
}
} // namespace Emergence::Celerity::WorldRendering2d

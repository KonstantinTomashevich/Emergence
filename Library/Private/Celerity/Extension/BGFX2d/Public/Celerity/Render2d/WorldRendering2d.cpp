#include <Celerity/PipelineBuilderMacros.hpp>
#include <Celerity/Render2d/BGFX/Rendering.hpp>
#include <Celerity/Render2d/BackendApi.hpp>
#include <Celerity/Render2d/BoundsCalculation2d.hpp>
#include <Celerity/Render2d/Camera2dComponent.hpp>
#include <Celerity/Render2d/Events.hpp>
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

    void ApplyViewportConfiguration () noexcept;

    void CollectVisibleObjects (UniqueId _cameraObjectId,
                                Math::Transform2d &_selectedCameraTransform,
                                Math::Vector2f &_selectedCameraHalfOrthographicSize) noexcept;

    WorldRenderer::Batch &GetBatch (uint16_t _layer, Memory::UniqueString _materialInstanceId) noexcept;

    void SubmitBatches (std::uint16_t _viewportNativeId,
                        const Math::Transform2d &_selectedCameraTransform,
                        const Math::Vector2f &_selectedCameraHalfOrthographicSize) noexcept;

    void PoolBatches () noexcept;

    FetchValueQuery fetchViewportByName;
    FetchAscendingRangeQuery fetchViewportBySortIndexAscending;
    FetchValueQuery fetchCameraById;

    FetchSequenceQuery fetchViewportAddedNormalEvent;
    FetchSequenceQuery fetchViewportAddedCustomEvent;
    FetchSequenceQuery fetchViewportChangedEvent;

    FetchValueQuery fetchTransformById;
    Transform2dWorldAccessor transformWorldAccessor;

    FetchShapeIntersectionQuery fetchVisibleRenderObjects;
    FetchValueQuery fetchLocalBoundsByRenderObjectId;
    FetchValueQuery fetchSpriteByObjectId;

    BGFX::RenderingBackend renderingBackend;
    Container::Vector<Batch> batches {Memory::Profiler::AllocationGroup::Top ()};
    Container::Vector<Batch> batchPool {Memory::Profiler::AllocationGroup::Top ()};
    Container::Vector<uint16_t> viewportOrder {Memory::Profiler::AllocationGroup::Top ()};
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
    : fetchViewportByName (FETCH_VALUE_1F (Viewport2d, name)),
      fetchViewportBySortIndexAscending (FETCH_ASCENDING_RANGE (Viewport2d, sortIndex)),
      fetchCameraById (FETCH_VALUE_1F (Camera2dComponent, objectId)),

      fetchViewportAddedNormalEvent (FETCH_SEQUENCE (Viewport2dAddedNormalEvent)),
      fetchViewportAddedCustomEvent (FETCH_SEQUENCE (Viewport2dAddedCustomToNormalEvent)),
      fetchViewportChangedEvent (FETCH_SEQUENCE (Viewport2dChangedNormalEvent)),

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
    ApplyViewportConfiguration ();

    for (auto viewportCursor = fetchViewportBySortIndexAscending.Execute (nullptr, nullptr);
         const auto *viewport = static_cast<const Viewport2d *> (*viewportCursor); ++viewportCursor)
    {
        Math::Transform2d selectedCameraTransform;
        Math::Vector2f selectedCameraHalfOrthographicSize {Math::Vector2f::ZERO};

        CollectVisibleObjects (viewport->cameraObjectId, selectedCameraTransform, selectedCameraHalfOrthographicSize);
        SubmitBatches (viewport->nativeId, selectedCameraTransform, selectedCameraHalfOrthographicSize);
        PoolBatches ();
        viewportOrder.emplace_back (viewport->nativeId);
    }

    if (!viewportOrder.empty ())
    {
        BGFX::RenderingBackend::SubmitViewOrder (viewportOrder);
        viewportOrder.clear ();
    }

    renderingBackend.EndFrame ();
}

void WorldRenderer::ApplyViewportConfiguration () noexcept
{
    auto applyViewportConfiguration = [this] (Memory::UniqueString _name)
    {
        auto cursor = fetchViewportByName.Execute (&_name);
        if (const auto *viewport = static_cast<const Viewport2d *> (*cursor))
        {
            BGFX::RenderingBackend::UpdateViewportConfiguration (*viewport);
        }
        else
        {
            // Viewport already deleted.
        }
    };

    for (auto eventCursor = fetchViewportAddedNormalEvent.Execute ();
         const auto *event = static_cast<const Viewport2dAddedNormalEvent *> (*eventCursor); ++eventCursor)
    {
        applyViewportConfiguration (event->name);
    }

    for (auto eventCursor = fetchViewportAddedCustomEvent.Execute ();
         const auto *event = static_cast<const Viewport2dAddedCustomToNormalEvent *> (*eventCursor); ++eventCursor)
    {
        applyViewportConfiguration (event->name);
    }

    for (auto eventCursor = fetchViewportChangedEvent.Execute ();
         const auto *event = static_cast<const Viewport2dChangedNormalEvent *> (*eventCursor); ++eventCursor)
    {
        applyViewportConfiguration (event->name);
    }
}

void WorldRenderer::CollectVisibleObjects (UniqueId _cameraObjectId,
                                           Math::Transform2d &_selectedCameraTransform,
                                           Math::Vector2f &_selectedCameraHalfOrthographicSize) noexcept
{
    auto cameraCursor = fetchCameraById.Execute (&_cameraObjectId);
    const auto *camera = static_cast<const Camera2dComponent *> (*cameraCursor);

    if (!camera)
    {
        return;
    }

    auto cameraTransformCursor = fetchTransformById.Execute (&_cameraObjectId);
    const auto *cameraTransform = static_cast<const Transform2dComponent *> (*cameraTransformCursor);

    if (!cameraTransform)
    {
        return;
    }

    _selectedCameraTransform = cameraTransform->GetVisualWorldTransform (transformWorldAccessor);
    const Render2dBackendConfig &backendConfig = Render2dBackend::GetCurrentConfig ();
    const float aspectRatio = static_cast<float> (backendConfig.width) / static_cast<float> (backendConfig.height);
    _selectedCameraHalfOrthographicSize = {camera->halfOrthographicSize * aspectRatio, camera->halfOrthographicSize};

    const Math::AxisAlignedBox2d localVisibilityBox {-_selectedCameraHalfOrthographicSize,
                                                     _selectedCameraHalfOrthographicSize};
    const Math::Matrix3x3f cameraTransformMatrix {_selectedCameraTransform};
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
                    if (sprite->visibilityMask & camera->visibilityMask)
                    {
                        GetBatch (sprite->layer, sprite->materialInstanceId)
                            .rects.emplace_back (BGFX::RectData {worldTransform, sprite->uv, sprite->halfSize});
                    }
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

void WorldRenderer::SubmitBatches (std::uint16_t _viewportNativeId,
                                   const Math::Transform2d &_selectedCameraTransform,
                                   const Math::Vector2f &_selectedCameraHalfOrthographicSize) noexcept
{
    BGFX::RenderingBackend::SubmitCamera (_viewportNativeId, _selectedCameraTransform,
                                          _selectedCameraHalfOrthographicSize);

    for (const Batch &batch : batches)
    {
        renderingBackend.SubmitMaterialInstance (batch.materialInstanceId);
        renderingBackend.SubmitRects (_viewportNativeId, batch.rects);
    }
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
    auto visualGroup = _pipelineBuilder.OpenVisualGroup ("WorldRendering2d");
    _pipelineBuilder.AddCheckpoint (Checkpoint::STARTED);
    _pipelineBuilder.AddCheckpoint (Checkpoint::FINISHED);
    _pipelineBuilder.AddTask (Memory::UniqueString {"WorldRenderer2d"}).SetExecutor<WorldRenderer> (_worldBounds);
}
} // namespace Emergence::Celerity::WorldRendering2d

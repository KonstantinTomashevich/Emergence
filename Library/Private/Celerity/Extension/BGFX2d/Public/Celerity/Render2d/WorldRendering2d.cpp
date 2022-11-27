#include <Celerity/Asset/Asset.hpp>
#include <Celerity/PipelineBuilderMacros.hpp>
#include <Celerity/Render2d/BoundsCalculation2d.hpp>
#include <Celerity/Render2d/Camera2dComponent.hpp>
#include <Celerity/Render2d/Events.hpp>
#include <Celerity/Render2d/Material2d.hpp>
#include <Celerity/Render2d/Material2dInstance.hpp>
#include <Celerity/Render2d/Render2dSingleton.hpp>
#include <Celerity/Render2d/RenderObject2dComponent.hpp>
#include <Celerity/Render2d/Rendering2d.hpp>
#include <Celerity/Render2d/Sprite2dComponent.hpp>
#include <Celerity/Render2d/Texture2d.hpp>
#include <Celerity/Render2d/Viewport2d.hpp>
#include <Celerity/Render2d/WorldRendering2d.hpp>
#include <Celerity/Transform/TransformComponent.hpp>
#include <Celerity/Transform/TransformWorldAccessor.hpp>

#include <Log/Log.hpp>

#include <Render/Backend/Renderer.hpp>

namespace Emergence::Celerity::WorldRendering2d
{
const Memory::UniqueString Checkpoint::STARTED {"WorldRendering2dStarted"};
const Memory::UniqueString Checkpoint::FINISHED {"WorldRendering2dFinished"};

struct RectData final
{
    Math::Transform2d transform;
    Math::AxisAlignedBox2d uv;
    Math::Vector2f halfSize;
};

struct Vertex final
{
    Math::Vector2f translation;
    Math::Vector2f uv;
};

static const Vertex QUAD_VERTICES[4u] = {
    {{-1.0f, 1.0f}, {0.0f, 0.0f}},
    {{1.0f, 1.0f}, {1.0f, 0.0f}},
    {{1.0f, -1.0f}, {1.0f, 1.0f}},
    {{-1.0f, -1.0f}, {0.0f, 1.0f}},
};

static const uint16_t QUAD_INDICES[6u] = {2u, 1u, 0u, 0u, 3u, 2u};

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
        Container::Vector<RectData> rects;
    };

    void ApplyViewportConfiguration (Render2dSingleton *_render) noexcept;

    void CollectVisibleObjects (const Viewport2d *_viewport,
                                Math::Transform2d &_selectedCameraTransform,
                                Math::Vector2f &_selectedCameraHalfOrthographicSize) noexcept;

    WorldRenderer::Batch &GetBatch (uint16_t _layer, Memory::UniqueString _materialInstanceId) noexcept;

    void SubmitBatch (Render2dSingleton *_render, const Viewport2d *_viewport, const Batch &_batch) noexcept;

    void SubmitRects (Render2dSingleton *_render,
                      const Viewport2d *_viewport,
                      const Render::Backend::Program &_program,
                      const Container::Vector<RectData> &_rects) noexcept;

    void PoolBatches () noexcept;

    ModifySingletonQuery modifyRenderSingleton;

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

    FetchValueQuery fetchAssetById;
    FetchValueQuery fetchMaterialInstanceById;
    FetchValueQuery fetchMaterialById;
    FetchValueQuery fetchTextureById;

    FetchValueQuery fetchUniformByAssetIdAndName;
    FetchValueQuery fetchUniformVector4fByInstanceId;
    FetchValueQuery fetchUniformMatrix3x3fByInstanceId;
    FetchValueQuery fetchUniformMatrix4x4fByInstanceId;
    FetchValueQuery fetchUniformSamplerByInstanceId;

    Container::Vector<Batch> batches {Memory::Profiler::AllocationGroup::Top ()};
    Container::Vector<Batch> batchPool {Memory::Profiler::AllocationGroup::Top ()};
    Container::Vector<const Render::Backend::Viewport *> viewportOrder {Memory::Profiler::AllocationGroup::Top ()};

    Render::Backend::VertexLayout vertexLayout;
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
    : modifyRenderSingleton (MODIFY_SINGLETON (Render2dSingleton)),

      fetchViewportByName (FETCH_VALUE_1F (Viewport2d, name)),
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

      fetchAssetById (FETCH_VALUE_1F (Asset, id)),
      fetchMaterialInstanceById (FETCH_VALUE_1F (Material2dInstance, assetId)),
      fetchMaterialById (FETCH_VALUE_1F (Material2d, assetId)),
      fetchTextureById (FETCH_VALUE_1F (Texture2d, assetId)),

      fetchUniformByAssetIdAndName (FETCH_VALUE_2F (Uniform2d, assetId, name)),
      fetchUniformVector4fByInstanceId (FETCH_VALUE_1F (UniformVector4fValue, assetId)),
      fetchUniformMatrix3x3fByInstanceId (FETCH_VALUE_1F (UniformMatrix3x3fValue, assetId)),
      fetchUniformMatrix4x4fByInstanceId (FETCH_VALUE_1F (UniformMatrix4x4fValue, assetId)),
      fetchUniformSamplerByInstanceId (FETCH_VALUE_1F (UniformSamplerValue, assetId)),

      vertexLayout (Render::Backend::VertexLayoutBuilder {}
                        .Begin ()
                        .Add (Render::Backend::Attribute::POSITION, Render::Backend::AttributeType::FLOAT, 2u)
                        .Add (Render::Backend::Attribute::SAMPLER_COORD_0, Render::Backend::AttributeType::FLOAT, 2u)
                        .End ())
{
    _constructor.DependOn (Rendering2d::Checkpoint::STARTED);
    _constructor.DependOn (Checkpoint::STARTED);
    _constructor.DependOn (BoundsCalculation2d::Checkpoint::FINISHED);
    _constructor.MakeDependencyOf (Checkpoint::FINISHED);
    _constructor.MakeDependencyOf (Rendering2d::Checkpoint::FINISHED);
}

void WorldRenderer::Execute () noexcept
{
    auto renderCursor = modifyRenderSingleton.Execute ();
    auto *render = static_cast<Render2dSingleton *> (*renderCursor);
    ApplyViewportConfiguration (render);

    for (auto viewportCursor = fetchViewportBySortIndexAscending.Execute (nullptr, nullptr);
         const auto *viewport = static_cast<const Viewport2d *> (*viewportCursor); ++viewportCursor)
    {
        Math::Transform2d selectedCameraTransform;
        Math::Vector2f selectedCameraHalfOrthographicSize {Math::Vector2f::ZERO};

        CollectVisibleObjects (viewport, selectedCameraTransform, selectedCameraHalfOrthographicSize);
        viewport->viewport.SubmitOrthographicView (selectedCameraTransform, selectedCameraHalfOrthographicSize);

        for (const Batch &batch : batches)
        {
            SubmitBatch (render, viewport, batch);
        }

        PoolBatches ();
        render->renderer.Touch (viewport->viewport);
        // Technically speaking, collecting viewports like that is dangerous and may lead to memory corruption.
        // But in this case task holds fetch access to Viewport2d and modify access to RenderBackend API, therefore
        // in this particular case this operation is safe. But we should use such tricks only if there is no other way.
        viewportOrder.emplace_back (&viewport->viewport);
    }

    if (!viewportOrder.empty ())
    {
        render->renderer.SubmitViewportOrder (viewportOrder);
        viewportOrder.clear ();
    }

    render->renderer.SubmitFrame ();
}

void WorldRenderer::ApplyViewportConfiguration (Render2dSingleton *_render) noexcept
{
    auto applyViewportConfiguration = [this, _render] (Memory::UniqueString _name, bool _initialize)
    {
        auto cursor = fetchViewportByName.Execute (&_name);
        if (const auto *viewport = static_cast<const Viewport2d *> (*cursor))
        {
            if (_initialize)
            {
                viewport->viewport = Render::Backend::Viewport {_render->renderer};
            }

            viewport->viewport.SubmitConfiguration (viewport->x, viewport->y, viewport->width, viewport->height,
                                                    Render::Backend::ViewportSortMode::SEQUENTIAL,
                                                    viewport->clearColor);
        }
        else
        {
            // Viewport already deleted.
        }
    };

    for (auto eventCursor = fetchViewportAddedNormalEvent.Execute ();
         const auto *event = static_cast<const Viewport2dAddedNormalEvent *> (*eventCursor); ++eventCursor)
    {
        applyViewportConfiguration (event->name, true);
    }

    for (auto eventCursor = fetchViewportAddedCustomEvent.Execute ();
         const auto *event = static_cast<const Viewport2dAddedCustomToNormalEvent *> (*eventCursor); ++eventCursor)
    {
        applyViewportConfiguration (event->name, true);
    }

    for (auto eventCursor = fetchViewportChangedEvent.Execute ();
         const auto *event = static_cast<const Viewport2dChangedNormalEvent *> (*eventCursor); ++eventCursor)
    {
        applyViewportConfiguration (event->name, false);
    }
}

void WorldRenderer::CollectVisibleObjects (const Viewport2d *_viewport,
                                           Math::Transform2d &_selectedCameraTransform,
                                           Math::Vector2f &_selectedCameraHalfOrthographicSize) noexcept
{
    auto cameraCursor = fetchCameraById.Execute (&_viewport->cameraObjectId);
    const auto *camera = static_cast<const Camera2dComponent *> (*cameraCursor);

    if (!camera)
    {
        return;
    }

    auto cameraTransformCursor = fetchTransformById.Execute (&_viewport->cameraObjectId);
    const auto *cameraTransform = static_cast<const Transform2dComponent *> (*cameraTransformCursor);

    if (!cameraTransform)
    {
        return;
    }

    _selectedCameraTransform = cameraTransform->GetVisualWorldTransform (transformWorldAccessor);
    const float aspectRatio = static_cast<float> (_viewport->width) / static_cast<float> (_viewport->height);
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
                            .rects.emplace_back (RectData {worldTransform, sprite->uv, sprite->halfSize});
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
            next, Batch {_layer, _materialInstanceId, Container::Vector<RectData> {batches.get_allocator ()}});
    }

    Batch &pooledBatch = batchPool.back ();
    batchPool.pop_back ();

    pooledBatch.layer = _layer;
    pooledBatch.materialInstanceId = _materialInstanceId;
    pooledBatch.rects.clear ();
    return *batches.emplace (next, std::move (pooledBatch));
}

void WorldRenderer::SubmitBatch (Render2dSingleton *_render, const Viewport2d *_viewport, const Batch &_batch) noexcept
{
    auto assetCursor = fetchAssetById.Execute (&_batch.materialInstanceId);
    const auto *asset = static_cast<const Asset *> (*assetCursor);

    if (!asset || asset->state != AssetState::READY)
    {
        EMERGENCE_LOG (WARNING, "Celerity::Render2d: Material instance \"", _batch.materialInstanceId,
                       "\" cannot be submitted as it is not loaded.");
        return;
    }

    auto materialInstanceCursor = fetchMaterialInstanceById.Execute (&_batch.materialInstanceId);
    const auto *materialInstance = static_cast<const Material2dInstance *> (*materialInstanceCursor);
    EMERGENCE_ASSERT (materialInstance);

    auto materialCursor = fetchMaterialById.Execute (&materialInstance->materialId);
    const auto *material = static_cast<const Material2d *> (*materialCursor);
    EMERGENCE_ASSERT (material);

    struct
    {
        Memory::UniqueString assetId;
        Memory::UniqueString name;
    } uniformQuery;

    for (auto valueCursor = fetchUniformVector4fByInstanceId.Execute (&_batch.materialInstanceId);
         const auto *value = static_cast<const UniformVector4fValue *> (*valueCursor); ++valueCursor)
    {
        uniformQuery.assetId = material->assetId;
        uniformQuery.name = value->uniformName;

        if (auto uniformCursor = fetchUniformByAssetIdAndName.Execute (&uniformQuery);
            const auto *uniform = static_cast<const Uniform2d *> (*uniformCursor))
        {
            uniform->uniform.SetVector4f (value->value);
        }
        else
        {
            EMERGENCE_LOG (WARNING, "Celerity::Render2d: Material instance uniform value \"", _batch.materialInstanceId, ".",
                           value->uniformName, "\" cannot be submitted as it is not registered in material.");
        }
    }

    for (auto valueCursor = fetchUniformMatrix3x3fByInstanceId.Execute (&_batch.materialInstanceId);
         const auto *value = static_cast<const UniformMatrix3x3fValue *> (*valueCursor); ++valueCursor)
    {
        uniformQuery.assetId = material->assetId;
        uniformQuery.name = value->uniformName;

        if (auto uniformCursor = fetchUniformByAssetIdAndName.Execute (&uniformQuery);
            const auto *uniform = static_cast<const Uniform2d *> (*uniformCursor))
        {
            uniform->uniform.SetMatrix3x3f (value->value);
        }
        else
        {
            EMERGENCE_LOG (WARNING, "Celerity::Render2d: Material instance uniform value \"", _batch.materialInstanceId, ".",
                           value->uniformName, "\" cannot be submitted as it is not registered in material.");
        }
    }

    for (auto valueCursor = fetchUniformMatrix4x4fByInstanceId.Execute (&_batch.materialInstanceId);
         const auto *value = static_cast<const UniformMatrix4x4fValue *> (*valueCursor); ++valueCursor)
    {
        uniformQuery.assetId = material->assetId;
        uniformQuery.name = value->uniformName;

        if (auto uniformCursor = fetchUniformByAssetIdAndName.Execute (&uniformQuery);
            const auto *uniform = static_cast<const Uniform2d *> (*uniformCursor))
        {
            uniform->uniform.SetMatrix4x4f (value->value);
        }
        else
        {
            EMERGENCE_LOG (WARNING, "Celerity::Render2d: Material instance uniform value \"", _batch.materialInstanceId, ".",
                           value->uniformName, "\" cannot be submitted as it is not registered in material.");
        }
    }

    for (auto valueCursor = fetchUniformSamplerByInstanceId.Execute (&_batch.materialInstanceId);
         const auto *value = static_cast<const UniformSamplerValue *> (*valueCursor); ++valueCursor)
    {
        auto textureAssetCursor = fetchAssetById.Execute (&value->textureId);
        const auto *textureAsset = static_cast<const Asset *> (*textureAssetCursor);

        if (!textureAsset || textureAsset->state != AssetState::READY)
        {
            EMERGENCE_LOG (WARNING, "Celerity::Render2d: Material instance uniform value \"", _batch.materialInstanceId, ".",
                           value->uniformName,
                           "\" cannot be submitted as required texture is not loaded. Skipping material submit.");
            return;
        }

        auto textureCursor = fetchTextureById.Execute (&value->textureId);
        const auto *texture = static_cast<const Texture2d *> (*textureCursor);
        EMERGENCE_ASSERT (texture);

        uniformQuery.assetId = material->assetId;
        uniformQuery.name = value->uniformName;

        if (auto uniformCursor = fetchUniformByAssetIdAndName.Execute (&uniformQuery);
            const auto *uniform = static_cast<const Uniform2d *> (*uniformCursor))
        {
            uniform->uniform.SetSampler (uniform->textureStage, texture->texture);
        }
        else
        {
            EMERGENCE_LOG (WARNING, "Celerity::Render2d: Material instance uniform value \"", _batch.materialInstanceId, ".",
                           value->uniformName, "\" cannot be submitted as it is not registered in material.");
        }
    }

    SubmitRects (_render, _viewport, material->program, _batch.rects);
}

void WorldRenderer::SubmitRects (
    Render2dSingleton *_render,
    const Viewport2d *_viewport,
    const Render::Backend::Program &_program,
    const Container::Vector<Emergence::Celerity::WorldRendering2d::RectData> &_rects) noexcept
{
    const auto totalVertices = static_cast<uint32_t> (_rects.size () * 4u);
    const auto totalIndices = static_cast<uint32_t> (_rects.size () * 6u);

    const uint32_t availableVertices =
        Render::Backend::TransientVertexBuffer::TruncateSizeToAvailability (totalVertices, vertexLayout);

    const uint32_t availableIndices =
        Render::Backend::TransientIndexBuffer::TruncateSizeToAvailability (totalIndices, false);

    if (availableVertices != totalVertices || availableIndices != totalIndices)
    {
        EMERGENCE_LOG (WARNING,
                       "Celerity::Render2d: Unable to submit all rects due to being unable to allocate buffers.");
    }

    const uint32_t maxRects = std::min (availableVertices / 4u, availableIndices / 6u);
    const uint32_t size = std::min (maxRects, static_cast<uint32_t> (_rects.size ()));

    Render::Backend::TransientVertexBuffer vertexBuffer {totalVertices, vertexLayout};
    Render::Backend::TransientIndexBuffer indexBuffer {totalIndices, false};

    for (uint32_t index = 0u; index < size; ++index)
    {
        const RectData &rect = _rects[index];
        const Math::Matrix3x3f transformMatrix {rect.transform};
        Vertex *vertices = reinterpret_cast<Vertex *> (vertexBuffer.GetData ()) + static_cast<ptrdiff_t> (index * 4u);

        for (uint32_t vertexIndex = 0u; vertexIndex < 4u; ++vertexIndex)
        {
            Vertex &vertex = vertices[vertexIndex];
            const Math::Vector2f localPoint = QUAD_VERTICES[vertexIndex].translation * rect.halfSize;
            const Math::Vector3f globalPoint3f = transformMatrix * Math::Vector3f {localPoint.x, localPoint.y, 1.0f};

            vertex.translation.x = globalPoint3f.x;
            vertex.translation.y = globalPoint3f.y;

            vertex.uv.x = rect.uv.min.x + QUAD_VERTICES[vertexIndex].uv.x * (rect.uv.max.x - rect.uv.min.x);
            vertex.uv.y = rect.uv.min.y + QUAD_VERTICES[vertexIndex].uv.y * (rect.uv.max.y - rect.uv.min.y);
        }

        uint16_t *indices = reinterpret_cast<uint16_t *> (indexBuffer.GetData ()) + static_cast<ptrdiff_t> (index * 6u);
        for (uint32_t indexIndex = 0u; indexIndex < 6u; ++indexIndex)
        {
            indices[indexIndex] = static_cast<uint16_t> (QUAD_INDICES[indexIndex] + index * 4u);
        }
    }

    _render->renderer.SetState (Render::Backend::STATE_WRITE_R | Render::Backend::STATE_WRITE_G |
                                Render::Backend::STATE_WRITE_B | Render::Backend::STATE_WRITE_A |
                                Render::Backend::STATE_BLEND_ALPHA | Render::Backend::STATE_CULL_CW |
                                Render::Backend::STATE_WRITE_MSAA);

    _render->renderer.SubmitGeometry (_viewport->viewport, _program, vertexBuffer, indexBuffer);
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

#include <limits>

#include <Celerity/Asset/Asset.hpp>
#include <Celerity/PipelineBuilderMacros.hpp>
#include <Celerity/Render/2d/Batching2d.hpp>
#include <Celerity/Render/2d/Batching2dSingleton.hpp>
#include <Celerity/Render/2d/Camera2dComponent.hpp>
#include <Celerity/Render/2d/DebugShape2dComponent.hpp>
#include <Celerity/Render/2d/RenderObject2dComponent.hpp>
#include <Celerity/Render/2d/Sprite2dComponent.hpp>
#include <Celerity/Render/2d/WorldRendering2d.hpp>
#include <Celerity/Render/Foundation/MaterialInstanceSubmitter.hpp>
#include <Celerity/Render/Foundation/RenderFoundationSingleton.hpp>
#include <Celerity/Render/Foundation/RenderPipelineFoundation.hpp>
#include <Celerity/Render/Foundation/Viewport.hpp>
#include <Celerity/Transform/TransformComponent.hpp>
#include <Celerity/Transform/TransformWorldAccessor.hpp>

#include <Log/Log.hpp>

#include <Math/Constants.hpp>
#include <Math/Scalar.hpp>

#include <Render/Backend/Renderer.hpp>

namespace Emergence::Celerity::WorldRendering2d
{
const Memory::UniqueString Checkpoint::STARTED {"WorldRendering2d::Started"};
const Memory::UniqueString Checkpoint::FINISHED {"WorldRendering2d::Finished"};

struct RectVertex final
{
    Math::Vector2f translation;
    Math::Vector2f uv;
};

struct LineVertex final
{
    Math::Vector2f translation;
};

static const RectVertex QUAD_VERTICES[4u] = {
    {{-1.0f, 1.0f}, {0.0f, 0.0f}},
    {{1.0f, 1.0f}, {1.0f, 0.0f}},
    {{1.0f, -1.0f}, {1.0f, 1.0f}},
    {{-1.0f, -1.0f}, {0.0f, 1.0f}},
};

static const std::uint16_t QUAD_INDICES[6u] = {2u, 1u, 0u, 0u, 3u, 2u};

class WorldRenderer final : public TaskExecutorBase<WorldRenderer>
{
public:
    WorldRenderer (TaskConstructor &_constructor) noexcept;

    void Execute () noexcept;

private:
    void SubmitSprites (Render::Backend::SubmissionAgent &_agent,
                        const Viewport *_viewport,
                        const Render::Backend::ProgramId &_programId,
                        const Batch2d &_batch) noexcept;

    void SubmitDebugShapes (Render::Backend::SubmissionAgent &_agent,
                            const Viewport *_viewport,
                            const Render::Backend::ProgramId &_programId,
                            const Batch2d &_batch) noexcept;

    ModifySingletonQuery modifyBatching;
    FetchSingletonQuery fetchRenderFoundation;
    FetchValueQuery fetchViewportByName;

    FetchValueQuery fetchTransformById;
    Transform2dWorldAccessor transformWorldAccessor;

    FetchValueQuery fetchSpriteBySpriteId;
    FetchValueQuery fetchDebugShapeByDebugShapeId;

    Render::Backend::VertexLayout rectVertexLayout;
    Render::Backend::VertexLayout lineVertexLayout;

    MaterialInstanceSubmitter materialInstanceSubmitter;
};

WorldRenderer::WorldRenderer (TaskConstructor &_constructor) noexcept
    : TaskExecutorBase (_constructor),

      modifyBatching (MODIFY_SINGLETON (Batching2dSingleton)),
      fetchRenderFoundation (FETCH_SINGLETON (RenderFoundationSingleton)),
      fetchViewportByName (FETCH_VALUE_1F (Viewport, name)),

      fetchTransformById (FETCH_VALUE_1F (Transform2dComponent, objectId)),
      transformWorldAccessor (_constructor),

      fetchSpriteBySpriteId (FETCH_VALUE_1F (Sprite2dComponent, spriteId)),
      fetchDebugShapeByDebugShapeId (FETCH_VALUE_1F (DebugShape2dComponent, debugShapeId)),

      rectVertexLayout (
          Render::Backend::VertexLayoutBuilder {}
              .Begin ()
              .Add (Render::Backend::Attribute::POSITION, Render::Backend::AttributeType::FLOAT, 2u)
              .Add (Render::Backend::Attribute::SAMPLER_COORD_0, Render::Backend::AttributeType::FLOAT, 2u)
              .End ()),

      lineVertexLayout (Render::Backend::VertexLayoutBuilder {}
                            .Begin ()
                            .Add (Render::Backend::Attribute::POSITION, Render::Backend::AttributeType::FLOAT, 2u)
                            .End ()),

      materialInstanceSubmitter (_constructor)
{
    _constructor.DependOn (Batching2d::Checkpoint::FINISHED);
    _constructor.DependOn (Checkpoint::STARTED);
    _constructor.MakeDependencyOf (Checkpoint::FINISHED);
    _constructor.MakeDependencyOf (RenderPipelineFoundation::Checkpoint::RENDER_FINISHED);
}

void WorldRenderer::Execute () noexcept
{
    auto batchingCursor = modifyBatching.Execute ();
    auto *batching = static_cast<Batching2dSingleton *> (*batchingCursor);

    auto renderFoundationCursor = fetchRenderFoundation.Execute ();
    const auto *renderFoundation = static_cast<const RenderFoundationSingleton *> (*renderFoundationCursor);
    Render::Backend::SubmissionAgent agent = renderFoundation->renderer.BeginSubmission ();

    for (const ViewportInfoContainer &viewportInfo : batching->viewports)
    {
        auto viewportCursor = fetchViewportByName.Execute (&viewportInfo.name);
        const auto *viewport = static_cast<const Viewport *> (*viewportCursor);

        if (!viewport)
        {
            continue;
        }

        EMERGENCE_ASSERT (viewport->sortMode == Render::Backend::ViewportSortMode::SEQUENTIAL);
        viewport->viewport.SubmitOrthographicView (viewportInfo.cameraTransform,
                                                   viewportInfo.cameraHalfOrthographicSize);

        for (const Batch2d &batch : viewportInfo.batches)
        {
            if (Container::Optional<Render::Backend::ProgramId> programId =
                    materialInstanceSubmitter.Submit (agent, batch.materialInstanceId))
            {
                SubmitSprites (agent, viewport, programId.value (), batch);
                SubmitDebugShapes (agent, viewport, programId.value (), batch);
            }
        }

        agent.Touch (viewport->viewport.GetId ());
    }

    batching->Reset ();
}

void WorldRenderer::SubmitSprites (Render::Backend::SubmissionAgent &_agent,
                                   const Viewport *_viewport,
                                   const Render::Backend::ProgramId &_programId,
                                   const Batch2d &_batch) noexcept
{
    if (_batch.sprites.empty ())
    {
        return;
    }

    const auto totalVertices = static_cast<std::uint32_t> (_batch.sprites.size () * 4u);
    const auto totalIndices = static_cast<std::uint32_t> (_batch.sprites.size () * 6u);

    const std::uint32_t availableVertices =
        Render::Backend::TransientVertexBuffer::TruncateSizeToAvailability (totalVertices, rectVertexLayout);

    const std::uint32_t availableIndices =
        Render::Backend::TransientIndexBuffer::TruncateSizeToAvailability (totalIndices, false);

    if (availableVertices != totalVertices || availableIndices != totalIndices)
    {
        EMERGENCE_LOG (WARNING, "Render2d: Unable to submit all rects due to being unable to allocate buffers.");
    }

    const std::uint32_t maxRects = std::min (availableVertices / 4u, availableIndices / 6u);
    const std::uint32_t size = std::min (maxRects, static_cast<std::uint32_t> (_batch.sprites.size ()));

    Render::Backend::TransientVertexBuffer vertexBuffer {totalVertices, rectVertexLayout};
    Render::Backend::TransientIndexBuffer indexBuffer {totalIndices, false};

    for (std::uint32_t index = 0u; index < size; ++index)
    {
        auto spriteCursor = fetchSpriteBySpriteId.Execute (&_batch.sprites[index]);
        const auto *sprite = static_cast<const Sprite2dComponent *> (*spriteCursor);

        if (!sprite)
        {
            continue;
        }

        // Assert that neither material instance neither layer was changed after batching.
        EMERGENCE_ASSERT (sprite->materialInstanceId == _batch.materialInstanceId);
        EMERGENCE_ASSERT (sprite->layer == _batch.layer);

        auto transformCursor = fetchTransformById.Execute (&sprite->objectId);
        const auto *transform = static_cast<const Transform2dComponent *> (*transformCursor);

        if (!transform)
        {
            continue;
        }

        const Math::Matrix3x3f transformMatrix {transform->GetVisualWorldTransform (transformWorldAccessor)};
        RectVertex *vertices =
            reinterpret_cast<RectVertex *> (vertexBuffer.GetData ()) + static_cast<ptrdiff_t> (index * 4u);

        for (std::uint32_t vertexIndex = 0u; vertexIndex < 4u; ++vertexIndex)
        {
            RectVertex &vertex = vertices[vertexIndex];
            const Math::Vector2f localPoint = QUAD_VERTICES[vertexIndex].translation * sprite->halfSize;
            const Math::Vector3f globalPoint3f = transformMatrix * Math::Vector3f {localPoint.x, localPoint.y, 1.0f};

            vertex.translation.x = globalPoint3f.x;
            vertex.translation.y = globalPoint3f.y;

            vertex.uv.x = sprite->uv.min.x + QUAD_VERTICES[vertexIndex].uv.x * (sprite->uv.max.x - sprite->uv.min.x);
            vertex.uv.y = sprite->uv.min.y + QUAD_VERTICES[vertexIndex].uv.y * (sprite->uv.max.y - sprite->uv.min.y);
        }

        std::uint16_t *indices =
            reinterpret_cast<std::uint16_t *> (indexBuffer.GetData ()) + static_cast<ptrdiff_t> (index * 6u);
        for (std::uint32_t indexIndex = 0u; indexIndex < 6u; ++indexIndex)
        {
            indices[indexIndex] = static_cast<std::uint16_t> (QUAD_INDICES[indexIndex] + index * 4u);
        }
    }

    _agent.SetState (Render::Backend::STATE_WRITE_R | Render::Backend::STATE_WRITE_G | Render::Backend::STATE_WRITE_B |
                     Render::Backend::STATE_WRITE_A | Render::Backend::STATE_BLEND_ALPHA |
                     Render::Backend::STATE_CULL_CW | Render::Backend::STATE_MSAA |
                     Render::Backend::STATE_PRIMITIVE_TRIANGLES);

    _agent.SubmitGeometry (_viewport->viewport.GetId (), _programId, vertexBuffer, indexBuffer);
}

void WorldRenderer::SubmitDebugShapes (Render::Backend::SubmissionAgent &_agent,
                                       const Viewport *_viewport,
                                       const Render::Backend::ProgramId &_programId,
                                       const Batch2d &_batch) noexcept
{
    if (_batch.debugShapes.empty ())
    {
        return;
    }

    constexpr std::size_t DEBUG_CIRCLE_POINT_COUNT = 16u;
    std::size_t lineCount = 0u;

    for (UniqueId debugShapeId : _batch.debugShapes)
    {
        auto shapeCursor = fetchDebugShapeByDebugShapeId.Execute (&debugShapeId);
        if (const auto *shape = static_cast<const DebugShape2dComponent *> (*shapeCursor))
        {
            switch (shape->shape.type)
            {
            case DebugShape2dType::BOX:
                lineCount += 4u;
                break;

            case DebugShape2dType::CIRCLE:
                lineCount += DEBUG_CIRCLE_POINT_COUNT;
                break;

            case DebugShape2dType::LINE:
                ++lineCount;
                break;
            }
        }
    }

    if (lineCount == 0u)
    {
        return;
    }

    const auto totalVertices = static_cast<std::uint32_t> (lineCount * 2u);
    const auto totalIndices = static_cast<std::uint32_t> (lineCount * 2u);

    const std::uint32_t availableVertices =
        Render::Backend::TransientVertexBuffer::TruncateSizeToAvailability (totalVertices, lineVertexLayout);

    const std::uint32_t availableIndices =
        Render::Backend::TransientIndexBuffer::TruncateSizeToAvailability (totalIndices, false);

    if (availableVertices != totalVertices || availableIndices != totalIndices)
    {
        EMERGENCE_LOG (WARNING, "Render2d: Unable to submit all lines due to being unable to allocate buffers.");
    }

    const std::uint32_t maxLines = std::min (availableVertices / 2u, availableIndices / 2u);
    const std::uint32_t size = std::min (maxLines, static_cast<std::uint32_t> (lineCount));

    Render::Backend::TransientVertexBuffer vertexBuffer {totalVertices, lineVertexLayout};
    Render::Backend::TransientIndexBuffer indexBuffer {totalIndices, false};

    std::size_t lineIndex = 0u;
    auto addLine =
        [size, &lineIndex, &vertexBuffer, &indexBuffer] (const Math::Vector2f &_start, const Math::Vector2f &_end)
    {
        if (lineIndex >= size)
        {
            return;
        }

        LineVertex *vertices =
            reinterpret_cast<LineVertex *> (vertexBuffer.GetData ()) + static_cast<ptrdiff_t> (lineIndex * 2u);

        vertices[0u].translation = _start;
        vertices[1u].translation = _end;

        std::uint16_t *indices =
            reinterpret_cast<std::uint16_t *> (indexBuffer.GetData ()) + static_cast<ptrdiff_t> (lineIndex * 2u);
        EMERGENCE_ASSERT (lineIndex * 2u + 1 < std::numeric_limits<std::uint16_t>::max ());
        indices[0u] = static_cast<std::uint16_t> (lineIndex * 2u);
        indices[1u] = static_cast<std::uint16_t> (lineIndex * 2u + 1u);
        ++lineIndex;
    };

    for (UniqueId debugShapeId : _batch.debugShapes)
    {
        auto shapeCursor = fetchDebugShapeByDebugShapeId.Execute (&debugShapeId);
        const auto *shape = static_cast<const DebugShape2dComponent *> (*shapeCursor);

        if (!shape)
        {
            continue;
        }

        // Assert that material instance was not changed after batching.
        EMERGENCE_ASSERT (shape->materialInstanceId == _batch.materialInstanceId);

        auto transformCursor = fetchTransformById.Execute (&shape->objectId);
        const auto *transform = static_cast<const Transform2dComponent *> (*transformCursor);

        if (!transform)
        {
            continue;
        }

        const Math::Matrix3x3f transformMatrix {transform->GetVisualWorldTransform (transformWorldAccessor)};
        const Math::Transform2d shapeLocalTransform {shape->translation, shape->rotation, Math::Vector2f::ONE};
        const Math::Matrix3x3f shapeWorldTransformMatrix = transformMatrix * Math::Matrix3x3f {shapeLocalTransform};

        auto transformPoint = [&shapeWorldTransformMatrix] (const Math::Vector2f &_point)
        {
            const Math::Vector3f point3 = shapeWorldTransformMatrix * Math::Vector3f {_point.x, _point.y, 1.0f};
            return Math::Vector2f {point3.x, point3.y};
        };

        switch (shape->shape.type)
        {
        case DebugShape2dType::BOX:
            for (std::size_t startVertexIndex = 0u; startVertexIndex < 4u; ++startVertexIndex)
            {
                const RectVertex &startVertex = QUAD_VERTICES[startVertexIndex];
                const RectVertex &endVertex = QUAD_VERTICES[(startVertexIndex + 1u) % 4u];

                const Math::Vector2f startPoint = startVertex.translation * shape->shape.boxHalfExtents;
                const Math::Vector2f endPoint = endVertex.translation * shape->shape.boxHalfExtents;
                addLine (transformPoint (startPoint), transformPoint (endPoint));
            }

            break;

        case DebugShape2dType::CIRCLE:
        {
            constexpr std::size_t POINT_COUNT = 16u;
            for (std::size_t startVertexIndex = 0u; startVertexIndex < POINT_COUNT; ++startVertexIndex)
            {
                const float startAngle =
                    static_cast<float> (startVertexIndex) * 2.0f * Math::PI / static_cast<float> (POINT_COUNT);

                const float endAngle = static_cast<float> ((startVertexIndex + 1u) % POINT_COUNT) * 2.0f * Math::PI /
                                       static_cast<float> (POINT_COUNT);

                const Math::Vector2f startPoint =
                    Math::Vector2f {Math::Cos (startAngle), Math::Sin (startAngle)} * shape->shape.circleRadius;

                const Math::Vector2f endPoint =
                    Math::Vector2f {Math::Cos (endAngle), Math::Sin (endAngle)} * shape->shape.circleRadius;
                addLine (transformPoint (startPoint), transformPoint (endPoint));
            }

            break;
        }

        case DebugShape2dType::LINE:
            addLine (transformPoint (Math::Vector2f::ZERO), transformPoint (shape->shape.lineEnd));
            break;
        }
    }

    _agent.SetState (Render::Backend::STATE_WRITE_R | Render::Backend::STATE_WRITE_G | Render::Backend::STATE_WRITE_B |
                     Render::Backend::STATE_WRITE_A | Render::Backend::STATE_BLEND_ALPHA |
                     Render::Backend::STATE_CULL_CW | Render::Backend::STATE_MSAA |
                     Render::Backend::STATE_PRIMITIVE_LINES);

    _agent.SubmitGeometry (_viewport->viewport.GetId (), _programId, vertexBuffer, indexBuffer);
}

void AddToNormalUpdate (PipelineBuilder &_pipelineBuilder) noexcept
{
    using namespace Memory::Literals;

    auto visualGroup = _pipelineBuilder.OpenVisualGroup ("WorldRendering2d");
    _pipelineBuilder.AddCheckpoint (Checkpoint::STARTED);
    _pipelineBuilder.AddCheckpoint (Checkpoint::FINISHED);
    _pipelineBuilder.AddTask ("WorldRenderer2d"_us).SetExecutor<WorldRenderer> ();
}
} // namespace Emergence::Celerity::WorldRendering2d

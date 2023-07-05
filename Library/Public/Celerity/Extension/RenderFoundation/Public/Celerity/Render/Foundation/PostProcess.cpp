#include <Celerity/PipelineBuilderMacros.hpp>
#include <Celerity/Render/Foundation/MaterialInstanceSubmitter.hpp>
#include <Celerity/Render/Foundation/PostProcess.hpp>
#include <Celerity/Render/Foundation/PostProcessRenderPass.hpp>
#include <Celerity/Render/Foundation/RenderFoundationSingleton.hpp>
#include <Celerity/Render/Foundation/RenderPipelineFoundation.hpp>
#include <Celerity/Render/Foundation/Viewport.hpp>

#include <Log/Log.hpp>

namespace Emergence::Celerity::PostProcess
{
struct Vertex final
{
    Math::Vector2f translation;
    Math::Vector2f uv;
};

class Renderer final : public TaskExecutorBase<Renderer>
{
public:
    Renderer (TaskConstructor &_constructor) noexcept;

    void Execute () noexcept;

private:
    FetchSingletonQuery fetchRenderFoundation;
    FetchAscendingRangeQuery fetchRenderPassesByNameAscending;
    FetchValueQuery fetchViewportByName;

    MaterialInstanceSubmitter materialInstanceSubmitter;

    Render::Backend::VertexLayout vertexLayout;
};

Renderer::Renderer (TaskConstructor &_constructor) noexcept
    : TaskExecutorBase (_constructor),

      fetchRenderFoundation (FETCH_SINGLETON (RenderFoundationSingleton)),
      fetchRenderPassesByNameAscending (FETCH_ASCENDING_RANGE (PostProcessRenderPass, name)),
      fetchViewportByName (FETCH_VALUE_1F (Viewport, name)),

      materialInstanceSubmitter (_constructor),

      vertexLayout (Render::Backend::VertexLayoutBuilder {}
                        .Begin ()
                        .Add (Render::Backend::Attribute::POSITION, Render::Backend::AttributeType::FLOAT, 2u)
                        .Add (Render::Backend::Attribute::SAMPLER_COORD_0, Render::Backend::AttributeType::FLOAT, 2u)
                        .End ())
{
    _constructor.DependOn (RenderPipelineFoundation::Checkpoint::RENDER_STARTED);
    _constructor.MakeDependencyOf (RenderPipelineFoundation::Checkpoint::RENDER_FINISHED);
}

void Renderer::Execute () noexcept
{
    auto renderFoundationCursor = fetchRenderFoundation.Execute ();
    const auto *renderFoundation = static_cast<const RenderFoundationSingleton *> (*renderFoundationCursor);
    Render::Backend::SubmissionAgent agent = renderFoundation->renderer.BeginSubmission ();

    for (auto passCursor = fetchRenderPassesByNameAscending.Execute (nullptr, nullptr);
         const auto *pass = static_cast<const PostProcessRenderPass *> (*passCursor); ++passCursor)
    {
        auto viewportCursor = fetchViewportByName.Execute (&pass->name);
        const auto *viewport = static_cast<const Viewport *> (*viewportCursor);

        if (!viewport)
        {
            continue;
        }

        const float aspectRatio = static_cast<float> (viewport->width) / static_cast<float> (viewport->height);
        const float halfHeight = 0.5f;
        const float halfWidth = aspectRatio * halfHeight;
        viewport->viewport.SubmitOrthographicView ({}, {halfWidth, halfHeight});

        Container::Optional<Render::Backend::ProgramId> programId =
            materialInstanceSubmitter.Submit (agent, pass->materialInstanceId);

        if (!programId)
        {
            continue;
        }

        Render::Backend::TransientVertexBuffer vertexBuffer {4u, vertexLayout};
        Render::Backend::TransientIndexBuffer indexBuffer {7u, false};

        auto *vertices = static_cast<Vertex *> (vertexBuffer.GetData ());
        vertices[0u] = {{-halfWidth, halfHeight}, {0.0f, 0.0f}};
        vertices[1u] = {{halfWidth, halfHeight}, {1.0f, 0.0f}};
        vertices[2u] = {{halfWidth, -halfHeight}, {1.0f, 1.0f}};
        vertices[3u] = {{-halfWidth, -halfHeight}, {0.0f, 1.0f}};

        auto *indices = static_cast<std::uint16_t *> (indexBuffer.GetData ());
        indices[0u] = 2u;
        indices[1u] = 1u;
        indices[2u] = 0u;
        indices[3u] = 0u;
        indices[4u] = 3u;
        indices[5u] = 2u;

        agent.SetState (Render::Backend::STATE_WRITE_R | Render::Backend::STATE_WRITE_G |
                        Render::Backend::STATE_WRITE_B | Render::Backend::STATE_WRITE_A |
                        Render::Backend::STATE_BLEND_ALPHA | Render::Backend::STATE_CULL_CW |
                        Render::Backend::STATE_MSAA | Render::Backend::STATE_PRIMITIVE_TRIANGLES);

        agent.SubmitGeometry (viewport->viewport.GetId (), programId.value (), vertexBuffer, indexBuffer);
    }
}

void AddToNormalUpdate (PipelineBuilder &_pipelineBuilder) noexcept
{
    using namespace Memory::Literals;
    auto visualGroup = _pipelineBuilder.OpenVisualGroup ("PostProcess");
    _pipelineBuilder.AddTask ("PostProcessRenderer"_us).SetExecutor<Renderer> ();
}
} // namespace Emergence::Celerity::PostProcess

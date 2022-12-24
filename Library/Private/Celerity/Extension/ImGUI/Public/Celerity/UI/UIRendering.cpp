#define _CRT_SECURE_NO_WARNINGS

#include <Celerity/Asset/Asset.hpp>
#include <Celerity/Asset/AssetManagerSingleton.hpp>
#include <Celerity/PipelineBuilderMacros.hpp>
#include <Celerity/Render/Foundation/Events.hpp>
#include <Celerity/Render/Foundation/Material.hpp>
#include <Celerity/Render/Foundation/RenderFoundationSingleton.hpp>
#include <Celerity/Render/Foundation/RenderPipelineFoundation.hpp>
#include <Celerity/Render/Foundation/Viewport.hpp>
#include <Celerity/UI/UIAssetPin.hpp>
#include <Celerity/UI/UIRenderPass.hpp>
#include <Celerity/UI/UIRendering.hpp>

#include <imgui.h>

#include <Log/Log.hpp>

namespace Emergence::Celerity::UIRendering
{
static const Memory::UniqueString MATERIAL_ID {"ImGUI"};
static const Memory::UniqueString TEXTURE_UNIFORM_NAME {"colorTexture"};

class UIRenderer final : public TaskExecutorBase<UIRenderer>
{
public:
    UIRenderer (TaskConstructor &_constructor) noexcept;

    void Execute ();

private:
    bool IsAssetPinExists () noexcept;

    void CreateAssetPin () noexcept;

    bool CheckMaterialAssetStatus () noexcept;

    bool FetchMaterialProgram (Render::Backend::ProgramId &_idOutput) noexcept;

    bool FetchTextureUniform (Render::Backend::UniformId &_idOutput, uint8_t &_textureStageOutput) noexcept;

    void ProcessRenderPass (const UIRenderPass *_renderPass,
                            Render::Backend::SubmissionAgent &_agent,
                            Render::Backend::ProgramId _programId,
                            Render::Backend::UniformId _textureUniformId,
                            uint8_t _textureStage) noexcept;

    FetchSingletonQuery fetchAssetManager;
    FetchSingletonQuery fetchRenderFoundation;

    FetchAscendingRangeQuery fetchAssetPin;
    InsertLongTermQuery insertAssetPin;

    FetchValueQuery fetchAssetById;
    FetchValueQuery fetchMaterialByAssetId;
    FetchValueQuery fetchUniformByAssetIdAndName;

    FetchAscendingRangeQuery fetchRenderPasses;
    FetchValueQuery fetchViewportById;

    Render::Backend::VertexLayout imGUIVertexLayout;
};

UIRenderer::UIRenderer (TaskConstructor &_constructor) noexcept
    : fetchAssetManager (FETCH_SINGLETON (AssetManagerSingleton)),
      fetchRenderFoundation (FETCH_SINGLETON (RenderFoundationSingleton)),

      fetchAssetPin (FETCH_ASCENDING_RANGE (UIAssetPin, assetUserId)),
      insertAssetPin (INSERT_LONG_TERM (UIAssetPin)),

      fetchAssetById (FETCH_VALUE_1F (Asset, id)),
      fetchMaterialByAssetId (FETCH_VALUE_1F (Material, assetId)),
      fetchUniformByAssetIdAndName (FETCH_VALUE_2F (Uniform, assetId, name)),

      fetchRenderPasses (FETCH_ASCENDING_RANGE (UIRenderPass, name)),
      fetchViewportById (FETCH_VALUE_1F (Viewport, name)),

      imGUIVertexLayout (Render::Backend::VertexLayoutBuilder {}
                        .Begin ()
                        .Add (Render::Backend::Attribute::POSITION, Render::Backend::AttributeType::FLOAT, 2u)
                        .Add (Render::Backend::Attribute::SAMPLER_COORD_0, Render::Backend::AttributeType::FLOAT, 2u)
                        .Add (Render::Backend::Attribute::COLOR_0, Render::Backend::AttributeType::UINT8, 4u, true)
                        .End ())
{
    _constructor.DependOn (RenderPipelineFoundation::Checkpoint::VIEWPORT_SYNC_FINISHED);
    _constructor.MakeDependencyOf (RenderPipelineFoundation::Checkpoint::RENDER_FINISHED);
}

void UIRenderer::Execute ()
{
    if (IsAssetPinExists ())
    {
        CreateAssetPin ();
    }

    Render::Backend::ProgramId programId;
    Render::Backend::UniformId textureUniformId;
    uint8_t textureStage;

    if (!CheckMaterialAssetStatus () || !FetchMaterialProgram (programId) ||
        !FetchTextureUniform (textureUniformId, textureStage))
    {
        return;
    }

    auto renderFoundationCursor = fetchRenderFoundation.Execute ();
    const auto *renderFoundation = static_cast<const RenderFoundationSingleton *> (*renderFoundationCursor);
    Render::Backend::SubmissionAgent agent = renderFoundation->renderer.BeginSubmission ();

    for (auto renderPassCursor = fetchRenderPasses.Execute (nullptr, nullptr);
         const auto *renderPass = static_cast<const UIRenderPass *> (*renderPassCursor); ++renderPassCursor)
    {
        ProcessRenderPass (renderPass, agent, programId, textureUniformId, textureStage);
    }
}

bool UIRenderer::IsAssetPinExists () noexcept
{
    auto cursor = fetchAssetPin.Execute (nullptr, nullptr);
    return *cursor;
}

void UIRenderer::CreateAssetPin () noexcept
{
    auto assetManagerCursor = fetchAssetManager.Execute ();
    const auto *assetManager = static_cast<const AssetManagerSingleton *> (*assetManagerCursor);

    auto cursor = insertAssetPin.Execute ();
    auto *assetPin = static_cast<UIAssetPin *> (++cursor);
    assetPin->assetUserId = assetManager->GenerateAssetUserId ();
    assetPin->materialId = MATERIAL_ID;
}

bool UIRenderer::CheckMaterialAssetStatus () noexcept
{
    if (auto cursor = fetchAssetById.Execute (&MATERIAL_ID); const auto *asset = static_cast<const Asset *> (*cursor))
    {
        return asset->state == AssetState::READY;
    }

    return false;
}

bool UIRenderer::FetchMaterialProgram (Render::Backend::ProgramId &_idOutput) noexcept
{
    if (auto cursor = fetchMaterialByAssetId.Execute (&MATERIAL_ID);
        const auto *material = static_cast<const Material *> (*cursor))
    {
        _idOutput = material->program.GetId ();
        return true;
    }

    return false;
}

bool UIRenderer::FetchTextureUniform (Render::Backend::UniformId &_idOutput, uint8_t &_textureStageOutput) noexcept
{
    struct
    {
        Memory::UniqueString assetId;
        Memory::UniqueString name;
    } uniformQuery;

    uniformQuery.assetId = MATERIAL_ID;
    uniformQuery.name = TEXTURE_UNIFORM_NAME;

    if (auto cursor = fetchUniformByAssetIdAndName.Execute (&uniformQuery);
        const auto *uniform = static_cast<const Uniform *> (*cursor))
    {
        EMERGENCE_ASSERT (uniform->type == Render::Backend::UniformType::SAMPLER);
        _idOutput = uniform->uniform.GetId ();
        _textureStageOutput = uniform->textureStage;
        return true;
    }

    return false;
}

void UIRenderer::ProcessRenderPass (const UIRenderPass *_renderPass,
                                    Render::Backend::SubmissionAgent &_agent,
                                    Render::Backend::ProgramId _programId,
                                    Render::Backend::UniformId _textureUniformId,
                                    uint8_t _textureStage) noexcept
{
    if (!_renderPass->nativeContext)
    {
        return;
    }

    auto viewportCursor = fetchViewportById.Execute (&_renderPass->name);
    const auto *viewport = static_cast<const Viewport *> (*viewportCursor);

    if (!viewport)
    {
        return;
    }

    EMERGENCE_ASSERT (viewport->sortMode == Render::Backend::ViewportSortMode::SEQUENTIAL);
    _agent.Touch (viewport->viewport.GetId ());
    ImGui::SetCurrentContext (static_cast<ImGuiContext *> (_renderPass->nativeContext));
    const ImDrawData *data = ImGui::GetDrawData ();

    if (!data)
    {
        return;
    }

    const float frameBufferWidth = data->DisplaySize.x * data->FramebufferScale.x;
    const float frameBufferHeight = data->DisplaySize.x * data->FramebufferScale.y;

    if (frameBufferWidth <= 0.0f || frameBufferHeight <= 0.0f)
    {
        return;
    }

    const Math::Vector2f halfOrthographicSize {data->DisplaySize.x * 0.5f, data->DisplaySize.y * 0.5f};
    const Math::Transform2d viewTransform {
        {data->DisplayPos.x + halfOrthographicSize.x, data->DisplayPos.y + halfOrthographicSize.y}, 0.0f, {1.0f, 1.0f}};
    viewport->viewport.SubmitOrthographicView (viewTransform, halfOrthographicSize);

    for (int listIndex = 0u; listIndex < data->CmdListsCount; ++listIndex)
    {
        const ImDrawList *drawList = data->CmdLists[listIndex];
        const auto totalVertices = static_cast<uint32_t> (drawList->VtxBuffer.size ());
        const auto totalIndices = static_cast<uint32_t> (drawList->IdxBuffer.size ());

        const uint32_t availableVertices =
            Render::Backend::TransientVertexBuffer::TruncateSizeToAvailability (totalVertices, imGUIVertexLayout);

        const uint32_t availableIndices =
            Render::Backend::TransientIndexBuffer::TruncateSizeToAvailability (totalIndices, false);

        if (availableVertices != totalVertices || availableIndices != totalIndices)
        {
            EMERGENCE_LOG (WARNING,
                           "Celerity::UI: Unable to submit all draw calls due to being unable to allocate buffers.");
            break;
        }

        Render::Backend::TransientVertexBuffer vertexBuffer {totalVertices, imGUIVertexLayout};
        Render::Backend::TransientIndexBuffer indexBuffer {totalIndices, false};

        memcpy (vertexBuffer.GetData (), drawList->VtxBuffer.begin (), totalVertices * sizeof (ImDrawVert));
        memcpy (indexBuffer.GetData (), drawList->IdxBuffer.begin (), totalIndices * sizeof (ImDrawIdx));

        for (const ImDrawCmd &command : drawList->CmdBuffer)
        {
            if (command.UserCallback)
            {
                command.UserCallback (drawList, &command);
                continue;
            }

            if (command.ElemCount == 0u)
            {
                continue;
            }

            const Math::Vector4f clipRect {
                (command.ClipRect.x - data->DisplayPos.x) * data->FramebufferScale.x,
                (command.ClipRect.y - data->DisplayPos.y) * data->FramebufferScale.y,
                (command.ClipRect.z - data->DisplayPos.x) * data->FramebufferScale.x,
                (command.ClipRect.w - data->DisplayPos.y) * data->FramebufferScale.y,
            };

            if (clipRect.x < frameBufferWidth && clipRect.y < frameBufferHeight && clipRect.z >= 0.0f &&
                clipRect.w >= 0.0f)
            {
                const uint32_t scissorX = viewport->x + std::max (static_cast<uint32_t> (clipRect.x), 0u);
                const uint32_t scissorY = viewport->y + std::max (static_cast<uint32_t> (clipRect.y), 0u);
                const uint32_t scissorWidth = std::max (static_cast<uint32_t> (clipRect.z - clipRect.x), 0u);
                const uint32_t scissorHeight = std::max (static_cast<uint32_t> (clipRect.w - clipRect.y), 0u);

                _agent.SetScissor (scissorX, scissorY,
                                   std::min (scissorWidth, viewport->width - (scissorX - viewport->x)),
                                   std::min (scissorHeight, viewport->height - (scissorY - viewport->y)));

                _agent.SetState (Render::Backend::STATE_WRITE_R | Render::Backend::STATE_WRITE_G |
                                 Render::Backend::STATE_WRITE_B | Render::Backend::STATE_WRITE_A |
                                 Render::Backend::STATE_MSAA | Render::Backend::STATE_BLEND_ALPHA);

                static_assert (sizeof (Render::Backend::TextureId) == sizeof (ImTextureID));
                _agent.SetSampler (_textureUniformId, _textureStage,
                                   *reinterpret_cast<const Render::Backend::TextureId *> (&command.TextureId));

                _agent.SubmitGeometry (viewport->viewport.GetId (), _programId, vertexBuffer, command.VtxOffset,
                                       totalVertices - command.VtxOffset, indexBuffer, command.IdxOffset,
                                       command.ElemCount);
            }
        }
    }
}

void AddToNormalUpdate (PipelineBuilder &_pipelineBuilder) noexcept
{
    using namespace Memory::Literals;

    auto visualGroup = _pipelineBuilder.OpenVisualGroup ("UIRendering");

    _pipelineBuilder.AddTask ("ClearUIRenderPassesAfterViewportRemoval"_us)
        .AS_CASCADE_REMOVER_1F (ViewportRemovedNormalEvent, UIRenderPass, name)
        .DependOn (RenderPipelineFoundation::Checkpoint::RENDER_STARTED)
        .MakeDependencyOf ("UIRenderer"_us);

    _pipelineBuilder.AddTask ("UIRenderer"_us).SetExecutor<UIRenderer> ();
}
} // namespace Emergence::Celerity::UIRendering

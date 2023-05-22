#define _CRT_SECURE_NO_WARNINGS

#include <cstring>

#include <Celerity/Asset/Asset.hpp>
#include <Celerity/Asset/Events.hpp>
#include <Celerity/Asset/StatefulAssetManagerBase.hpp>
#include <Celerity/Asset/UI/FontLoadingState.hpp>
#include <Celerity/Asset/UI/FontManagement.hpp>
#include <Celerity/Asset/UI/FontUtility.hpp>
#include <Celerity/PipelineBuilderMacros.hpp>
#include <Celerity/UI/Font.hpp>

#include <Job/Dispatcher.hpp>

#include <imgui.h>

#include <Log/Log.hpp>

#include <Math/Scalar.hpp>

namespace Emergence::Celerity::FontManagement
{
class Manager : public TaskExecutorBase<Manager>, public StatefulAssetManagerBase<Manager>
{
public:
    using AssetType = Font;

    using LoadingState = FontLoadingState;

    Manager (TaskConstructor &_constructor,
             ResourceProvider::ResourceProvider *_resourceProvider,
             const StandardLayout::Mapping &_stateUpdateEvent) noexcept;

private:
    friend class StatefulAssetManagerBase<Manager>;

    AssetState StartLoading (FontLoadingState *_loadingState) noexcept;

    AssetState TryFinishLoading (FontLoadingState *_loadingState) noexcept;

    void Unload (Memory::UniqueString _assetId) noexcept;

    InsertLongTermQuery insertFont;
    RemoveValueQuery removeFontById;

    ResourceProvider::ResourceProvider *resourceProvider;
};

Manager::Manager (TaskConstructor &_constructor,
                  ResourceProvider::ResourceProvider *_resourceProvider,
                  const StandardLayout::Mapping &_stateUpdateEvent) noexcept
    : StatefulAssetManagerBase (_constructor, _stateUpdateEvent),
      insertFont (INSERT_LONG_TERM (Font)),
      removeFontById (REMOVE_VALUE_1F (Font, assetId)),

      resourceProvider (_resourceProvider)
{
}

AssetState Manager::StartLoading (FontLoadingState *_loadingState) noexcept
{
    Job::Dispatcher::Global ().Dispatch (
        Job::Priority::BACKGROUND,
        [assetId {_loadingState->assetId}, cachedResourceProvider {resourceProvider},
         sharedState {_loadingState->sharedState}] ()
        {
            const char *sizeSeparator = strchr (*assetId, FONT_SIZE_SEPARATOR);
            EMERGENCE_ASSERT (sizeSeparator && *(sizeSeparator + 1u) != '\0');
            EMERGENCE_ASSERT (sizeSeparator != *assetId);
            Memory::UniqueString fontId {std::string_view {*assetId, static_cast<size_t> (sizeSeparator - *assetId)}};

            switch (cachedResourceProvider->LoadObject (FontAsset::Reflect ().mapping, fontId, &sharedState->asset))
            {
            case ResourceProvider::LoadingOperationResponse::SUCCESSFUL:
                break;

            case ResourceProvider::LoadingOperationResponse::NOT_FOUND:
                EMERGENCE_LOG (ERROR, "FontManagement: Unable to find font \"", fontId, "\".");
                sharedState->state = AssetState::MISSING;
                return;

            case ResourceProvider::LoadingOperationResponse::IO_ERROR:
                EMERGENCE_LOG (ERROR, "FontManagement: Failed to read font \"", fontId, "\".");
                sharedState->state = AssetState::CORRUPTED;
                return;

            case ResourceProvider::LoadingOperationResponse::WRONG_TYPE:
                sharedState->state = AssetState::CORRUPTED;
                return;
            }

            switch (cachedResourceProvider->LoadThirdPartyResource (
                sharedState->asset.fontId, sharedState->fontDataHeap, sharedState->fontDataSize, sharedState->fontData))
            {
            case ResourceProvider::LoadingOperationResponse::SUCCESSFUL:
                break;

            case ResourceProvider::LoadingOperationResponse::NOT_FOUND:
                EMERGENCE_LOG (ERROR, "FontManagement: Unable to find font source \"", sharedState->asset.fontId,
                               "\".");
                sharedState->state = AssetState::MISSING;
                return;

            case ResourceProvider::LoadingOperationResponse::IO_ERROR:
                EMERGENCE_LOG (ERROR, "FontManagement: Failed to read font source \"", sharedState->asset.fontId,
                               "\".");
                sharedState->state = AssetState::CORRUPTED;
                return;

            case ResourceProvider::LoadingOperationResponse::WRONG_TYPE:
                sharedState->state = AssetState::CORRUPTED;
                return;
            }

            sharedState->state = AssetState::READY;
        });

    return AssetState::LOADING;
}

AssetState Manager::TryFinishLoading (FontLoadingState *_loadingState) noexcept
{
    if (_loadingState->sharedState->state != AssetState::READY)
    {
        return _loadingState->sharedState->state;
    }

    const char *sizeSeparator = strchr (*_loadingState->assetId, FONT_SIZE_SEPARATOR);
    EMERGENCE_ASSERT (sizeSeparator && *(sizeSeparator + 1u) != '\0');
    auto fontSize = static_cast<float> (std::atof (sizeSeparator + 1u));
    EMERGENCE_ASSERT (!Math::NearlyEqual (fontSize, 0.0f));

    auto *fontAtlas = new (ImGui::MemAlloc (sizeof (ImFontAtlas))) ImFontAtlas ();
    // We need to reallocate data through ImGui allocation routine, so its ownership can be transferred to atlas.
    void *imGuiData = ImGui::MemAlloc (_loadingState->sharedState->fontDataSize);
    memcpy (imGuiData, _loadingState->sharedState->fontData, _loadingState->sharedState->fontDataSize);
    fontAtlas->AddFontFromMemoryTTF (imGuiData, static_cast<int> (_loadingState->sharedState->fontDataSize), fontSize);

    auto insertFontCursor = insertFont.Execute ();
    auto *font = static_cast<Font *> (++insertFontCursor);
    font->assetId = _loadingState->assetId;
    font->nativeHandle = fontAtlas;
    font->atlasTexture = BakeFontAtlas (fontAtlas);
    return AssetState::READY;
}

void Manager::Unload (Memory::UniqueString _assetId) noexcept
{
    if (auto fontCursor = removeFontById.Execute (&_assetId); fontCursor.ReadConst ())
    {
        ~fontCursor;
    }
}

void AddToNormalUpdate (PipelineBuilder &_pipelineBuilder,
                        ResourceProvider::ResourceProvider *_resourceProvider,
                        const AssetReferenceBindingEventMap &_eventMap) noexcept
{
    auto iterator = _eventMap.stateUpdate.find (Font::Reflect ().mapping);
    if (iterator == _eventMap.stateUpdate.end ())
    {
        EMERGENCE_LOG (WARNING,
                       "FontManagement: Task not registered, because Font is not found "
                       "in state update map. Perhaps it is not referenced by anything?");
        return;
    }

    auto visualGroup = _pipelineBuilder.OpenVisualGroup ("FontManagement");
    _pipelineBuilder.AddTask (Memory::UniqueString {"FontManager"})
        .SetExecutor<Manager> (_resourceProvider, iterator->second);
}
} // namespace Emergence::Celerity::FontManagement

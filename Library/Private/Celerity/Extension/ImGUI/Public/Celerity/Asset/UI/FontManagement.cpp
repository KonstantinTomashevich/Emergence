#define _CRT_SECURE_NO_WARNINGS

#include <cstring>

#include <Celerity/Asset/Asset.hpp>
#include <Celerity/Asset/AssetFileLoadingState.hpp>
#include <Celerity/Asset/Events.hpp>
#include <Celerity/Asset/TrivialFileAssetManager.hpp>
#include <Celerity/Asset/UI/Font.hpp>
#include <Celerity/Asset/UI/FontManagement.hpp>
#include <Celerity/Asset/UI/FontUtility.hpp>
#include <Celerity/PipelineBuilderMacros.hpp>

#include <imgui.h>

#include <Log/Log.hpp>

#include <Math/Scalar.hpp>

#include <SyntaxSugar/Time.hpp>

namespace Emergence::Celerity::FontManagement
{
class Manager : public TaskExecutorBase<Manager>, public TrivialFileAssetManager<Manager>
{
public:
    using AssetType = Font;

    constexpr static const char *const LOG_NAME = "FontManagement";

    Manager (TaskConstructor &_constructor,
             const Container::Vector<Memory::UniqueString> &_fontRootPaths,
             uint64_t _maxLoadingTimePerFrameNs,
             const StandardLayout::Mapping &_stateUpdateEvent) noexcept;

private:
    friend class TrivialFileAssetManager<Manager>;

    static Emergence::Container::String ExtractFilePath (Memory::UniqueString _assetId) noexcept;

    AssetState FinishLoading (AssetFileLoadingState *_loadingState) noexcept;

    InsertLongTermQuery insertFont;
};

Manager::Manager (TaskConstructor &_constructor,
                  const Container::Vector<Emergence::Memory::UniqueString> &_fontRootPaths,
                  uint64_t _maxLoadingTimePerFrameNs,
                  const StandardLayout::Mapping &_stateUpdateEvent) noexcept
    : TrivialFileAssetManager (_constructor, _fontRootPaths, _maxLoadingTimePerFrameNs, _stateUpdateEvent),
      insertFont (INSERT_LONG_TERM (Font))
{
}

Emergence::Container::String Manager::ExtractFilePath (Memory::UniqueString _assetId) noexcept
{
    const char *sizeSeparator = strchr (*_assetId, FONT_SIZE_SEPARATOR);
    EMERGENCE_ASSERT (sizeSeparator && *(sizeSeparator + 1u) != '\0');
    EMERGENCE_ASSERT (sizeSeparator != *_assetId);
    return {*_assetId, static_cast<size_t> (sizeSeparator - *_assetId)};
}

AssetState Manager::FinishLoading (AssetFileLoadingState *_loadingState) noexcept
{
    const char *sizeSeparator = strchr (*_loadingState->assetId, FONT_SIZE_SEPARATOR);
    EMERGENCE_ASSERT (sizeSeparator && *(sizeSeparator + 1u) != '\0');
    auto fontSize = static_cast<float> (std::atof (sizeSeparator + 1u));
    EMERGENCE_ASSERT (!Math::NearlyEqual (fontSize, 0.0f));

    auto *fontAtlas = new (ImGui::MemAlloc (sizeof (ImFontAtlas))) ImFontAtlas ();
    // We need to reallocate data through ImGui allocation routine, so its ownership can be transferred to atlas.
    void *imGuiData = ImGui::MemAlloc (_loadingState->size);
    memcpy (imGuiData, _loadingState->data, _loadingState->size);
    fontAtlas->AddFontFromMemoryTTF (imGuiData, static_cast<int> (_loadingState->size), fontSize);

    auto insertFontCursor = insertFont.Execute ();
    auto *font = static_cast<Font *> (++insertFontCursor);
    font->assetId = _loadingState->assetId;
    font->nativeHandle = fontAtlas;
    font->atlasTexture = BakeFontAtlas (fontAtlas);
    return AssetState::READY;
}

void AddToNormalUpdate (PipelineBuilder &_pipelineBuilder,
                        const Container::Vector<Memory::UniqueString> &_fontRootPaths,
                        uint64_t _maxLoadingTimePerFrameNs,
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
        .SetExecutor<Manager> (_fontRootPaths, _maxLoadingTimePerFrameNs, iterator->second);
}
} // namespace Emergence::Celerity::FontManagement

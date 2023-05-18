#define _CRT_SECURE_NO_WARNINGS

#include <cstring>

#include <Celerity/Asset/Asset.hpp>
#include <Celerity/Asset/Events.hpp>
#include <Celerity/Asset/StatefulAssetManagerBase.hpp>
#include <Celerity/Asset/UI/Font.hpp>
#include <Celerity/Asset/UI/FontLoadingState.hpp>
#include <Celerity/Asset/UI/FontManagement.hpp>
#include <Celerity/Asset/UI/FontUtility.hpp>
#include <Celerity/PipelineBuilderMacros.hpp>

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
             const Container::Vector<Memory::UniqueString> &_fontRootPaths,
             const StandardLayout::Mapping &_stateUpdateEvent) noexcept;

private:
    friend class StatefulAssetManagerBase<Manager>;

    AssetState StartLoading (FontLoadingState *_loadingState) noexcept;

    AssetState TryFinishLoading (FontLoadingState *_loadingState) noexcept;

    void Unload (Memory::UniqueString _assetId) noexcept;

    InsertLongTermQuery insertFont;
    RemoveValueQuery removeFontById;

    Container::Vector<Memory::UniqueString> rootPaths {Memory::Profiler::AllocationGroup::Top ()};
};

Manager::Manager (TaskConstructor &_constructor,
                  const Container::Vector<Emergence::Memory::UniqueString> &_fontRootPaths,
                  const StandardLayout::Mapping &_stateUpdateEvent) noexcept
    : StatefulAssetManagerBase (_constructor, _stateUpdateEvent),
      insertFont (INSERT_LONG_TERM (Font)),
      removeFontById (REMOVE_VALUE_1F (Font, assetId))
{
    rootPaths.reserve (_fontRootPaths.size ());
    for (Memory::UniqueString root : _fontRootPaths)
    {
        rootPaths.emplace_back (root);
    }
}

AssetState Manager::StartLoading (FontLoadingState *_loadingState) noexcept
{
    const char *sizeSeparator = strchr (*_loadingState->assetId, FONT_SIZE_SEPARATOR);
    EMERGENCE_ASSERT (sizeSeparator && *(sizeSeparator + 1u) != '\0');
    EMERGENCE_ASSERT (sizeSeparator != *_loadingState->assetId);
    Container::String fontFile {*_loadingState->assetId, static_cast<size_t> (sizeSeparator - *_loadingState->assetId)};

    for (Memory::UniqueString root : rootPaths)
    {
        std::filesystem::path filePath = EMERGENCE_BUILD_STRING (root, "/", fontFile);
        if (std::filesystem::exists (filePath))
        {
            Job::Dispatcher::Global ().Dispatch (
                Job::Priority::BACKGROUND,
                [sharedState {_loadingState->sharedState}, filePath] ()
                {
                    // TODO: Do not rely on 1-1 id mapping and add font settings?
                    FILE *file = std::fopen (filePath.generic_string ().c_str (), "rb");

                    if (!file)
                    {
                        EMERGENCE_LOG (ERROR, "FontManagement: Unable to open font file \"",
                                       filePath.generic_string<char, std::char_traits<char>, Memory::HeapSTD<char>> (),
                                       "\".");
                        sharedState->state = AssetState::CORRUPTED;
                        return;
                    }

                    fseek (file, 0u, SEEK_END);
                    sharedState->fontDataSize = static_cast<uint64_t> (ftell (file));
                    fseek (file, 0u, SEEK_SET);
                    sharedState->fontData = static_cast<uint8_t *> (
                        sharedState->fontDataHeap.Acquire (sharedState->fontDataSize, alignof (uint8_t)));

                    if (fread (sharedState->fontData, 1u, sharedState->fontDataSize, file) != sharedState->fontDataSize)
                    {
                        EMERGENCE_LOG (ERROR, "FontManagement: Unable to read font file \"",
                                       filePath.generic_string<char, std::char_traits<char>, Memory::HeapSTD<char>> (),
                                       "\".");
                        sharedState->state = AssetState::CORRUPTED;
                        return;
                    }

                    fclose (file);
                    sharedState->state = AssetState::READY;
                });

            return AssetState::LOADING;
        }
    }

    EMERGENCE_LOG (ERROR, "FontManagement: Unable to find font file for font \"", _loadingState->assetId, "\".");
    return AssetState::MISSING;
}

AssetState Manager::TryFinishLoading (FontLoadingState *_loadingState) noexcept
{
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
                        const Container::Vector<Memory::UniqueString> &_fontRootPaths,
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
        .SetExecutor<Manager> (_fontRootPaths, iterator->second);
}
} // namespace Emergence::Celerity::FontManagement

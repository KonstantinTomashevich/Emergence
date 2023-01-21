#define _CRT_SECURE_NO_WARNINGS

#include <Celerity/Asset/Asset.hpp>
#include <Celerity/Asset/AssetFileLoadingState.hpp>
#include <Celerity/Asset/AssetManagement.hpp>
#include <Celerity/Asset/Events.hpp>
#include <Celerity/Asset/Render/Foundation/TextureManagement.hpp>
#include <Celerity/Asset/TrivialFileAssetManager.hpp>
#include <Celerity/PipelineBuilderMacros.hpp>
#include <Celerity/Render/Foundation/Texture.hpp>

#include <Log/Log.hpp>

#include <SyntaxSugar/Time.hpp>

namespace Emergence::Celerity::TextureManagement
{
class Manager : public TaskExecutorBase<Manager>, public TrivialFileAssetManager<Manager>
{
public:
    using AssetType = Texture;

    constexpr static const char *const LOG_NAME = "TextureManagement";

    Manager (TaskConstructor &_constructor,
             const Container::Vector<Memory::UniqueString> &_textureRootPaths,
             uint64_t _maxLoadingTimePerFrameNs,
             const StandardLayout::Mapping &_stateUpdateEvent) noexcept;

private:
    friend class TrivialFileAssetManager<Manager>;

    static const char *ExtractFilePath (Memory::UniqueString _assetId) noexcept;

    AssetState FinishLoading (AssetFileLoadingState *_loadingState) noexcept;

    InsertLongTermQuery insertTexture;
};

Manager::Manager (TaskConstructor &_constructor,
                  const Container::Vector<Emergence::Memory::UniqueString> &_textureRootPaths,
                  uint64_t _maxLoadingTimePerFrameNs,
                  const StandardLayout::Mapping &_stateUpdateEvent) noexcept
    : TrivialFileAssetManager (_constructor, _textureRootPaths, _maxLoadingTimePerFrameNs, _stateUpdateEvent),
      insertTexture (INSERT_LONG_TERM (Texture))
{
}

const char *Manager::ExtractFilePath (Memory::UniqueString _assetId) noexcept
{
    return *_assetId;
}

AssetState Manager::FinishLoading (AssetFileLoadingState *_loadingState) noexcept
{
    Render::Backend::Texture nativeTexture {_loadingState->data, _loadingState->size};
    if (!nativeTexture.IsValid ())
    {
        EMERGENCE_LOG (ERROR, "TextureManagement: Failed to load texture \"", _loadingState->assetId, "\" from data.");
        return AssetState::CORRUPTED;
    }

    auto insertTextureCursor = insertTexture.Execute ();
    auto *texture = static_cast<Texture *> (++insertTextureCursor);
    texture->assetId = _loadingState->assetId;
    texture->texture = std::move (nativeTexture);
    return AssetState::READY;
}

void AddToNormalUpdate (PipelineBuilder &_pipelineBuilder,
                        const Container::Vector<Memory::UniqueString> &_textureRootPaths,
                        uint64_t _maxLoadingTimePerFrameNs,
                        const AssetReferenceBindingEventMap &_eventMap) noexcept
{
    auto iterator = _eventMap.stateUpdate.find (Texture::Reflect ().mapping);
    if (iterator == _eventMap.stateUpdate.end ())
    {
        EMERGENCE_LOG (WARNING,
                       "TextureManagement: Task not registered, because Texture is not found "
                       "in state update map. Perhaps it is not referenced by anything?");
        return;
    }

    auto visualGroup = _pipelineBuilder.OpenVisualGroup ("TextureManagement");
    _pipelineBuilder.AddTask (Memory::UniqueString {"TextureManager"})
        .SetExecutor<Manager> (_textureRootPaths, _maxLoadingTimePerFrameNs, iterator->second);
}
} // namespace Emergence::Celerity::TextureManagement

#define _CRT_SECURE_NO_WARNINGS

#include <fstream>

#include <Celerity/Asset/Asset.hpp>
#include <Celerity/Asset/AssetManagement.hpp>
#include <Celerity/Asset/Events.hpp>
#include <Celerity/Asset/Render/Foundation/TextureLoadingState.hpp>
#include <Celerity/Asset/Render/Foundation/TextureManagement.hpp>
#include <Celerity/Asset/StatefulAssetManagerBase.hpp>
#include <Celerity/PipelineBuilderMacros.hpp>
#include <Celerity/Render/Foundation/Texture.hpp>

#include <Job/Dispatcher.hpp>

#include <Log/Log.hpp>

#include <Serialization/Binary.hpp>
#include <Serialization/Yaml.hpp>

namespace Emergence::Celerity::TextureManagement
{
class Manager : public TaskExecutorBase<Manager>, public StatefulAssetManagerBase<Manager>
{
public:
    using AssetType = Texture;

    using LoadingState = TextureLoadingState;

    Manager (TaskConstructor &_constructor,
             const Container::Vector<Memory::UniqueString> &_textureRootPaths,
             const StandardLayout::Mapping &_stateUpdateEvent) noexcept;

private:
    friend class StatefulAssetManagerBase<Manager>;

    AssetState StartLoading (TextureLoadingState *_loadingState) noexcept;

    AssetState TryFinishLoading (TextureLoadingState *_loadingState) noexcept;

    void Unload (Memory::UniqueString _assetId) noexcept;

    InsertLongTermQuery insertTexture;
    RemoveValueQuery removeTextureById;

    Container::Vector<Memory::UniqueString> rootPaths {Memory::Profiler::AllocationGroup::Top ()};
};

Manager::Manager (TaskConstructor &_constructor,
                  const Container::Vector<Emergence::Memory::UniqueString> &_textureRootPaths,
                  const StandardLayout::Mapping &_stateUpdateEvent) noexcept
    : StatefulAssetManagerBase<Manager> (_constructor, _stateUpdateEvent),
      insertTexture (INSERT_LONG_TERM (Texture)),
      removeTextureById (REMOVE_VALUE_1F (Texture, assetId))
{
    rootPaths.reserve (_textureRootPaths.size ());
    for (Memory::UniqueString root : _textureRootPaths)
    {
        rootPaths.emplace_back (root);
    }
}

AssetState Manager::StartLoading (TextureLoadingState *_loadingState) noexcept
{
    std::filesystem::path settingsPath;
    Memory::UniqueString selectedRoot;
    bool binary;
    bool found = false;

    for (Memory::UniqueString root : rootPaths)
    {
        std::filesystem::path binaryPath = EMERGENCE_BUILD_STRING (root, "/", _loadingState->assetId, ".settings.bin");
        if (std::filesystem::exists (binaryPath))
        {
            settingsPath = binaryPath;
            selectedRoot = root;
            binary = true;
            found = true;
            break;
        }

        std::filesystem::path yamlPath = EMERGENCE_BUILD_STRING (root, "/", _loadingState->assetId, ".settings.yaml");
        if (std::filesystem::exists (yamlPath))
        {
            settingsPath = yamlPath;
            selectedRoot = root;
            binary = false;
            found = true;
            break;
        }
    }

    if (found)
    {
        Job::Dispatcher::Global ().Dispatch (
            Job::Priority::BACKGROUND,
            [assetId {_loadingState->assetId}, sharedState {_loadingState->sharedState}, settingsPath, selectedRoot,
             binary] ()
            {
                if (binary)
                {
                    std::ifstream input {settingsPath, std::ios::binary};
                    if (!Serialization::Binary::DeserializeObject (
                            input, &sharedState->settings, Render::Backend::TextureSettings::Reflect ().mapping, {}))
                    {
                        EMERGENCE_LOG (
                            ERROR, "TextureManagement: Unable to load texture settings from \"",
                            settingsPath.generic_string<char, std::char_traits<char>, Memory::HeapSTD<char>> (), "\".");
                        sharedState->state = AssetState::CORRUPTED;
                        return;
                    }
                }
                else
                {
                    std::ifstream input {settingsPath};
                    if (!Serialization::Yaml::DeserializeObject (
                            input, &sharedState->settings, Render::Backend::TextureSettings::Reflect ().mapping, {}))
                    {
                        EMERGENCE_LOG (
                            ERROR, "TextureManagement: Unable to load texture settings from \"",
                            settingsPath.generic_string<char, std::char_traits<char>, Memory::HeapSTD<char>> (), "\".");
                        sharedState->state = AssetState::CORRUPTED;
                        return;
                    }
                }

                // TODO: Do not rely on 1-1 id mapping and add texture id field to settings?
                std::filesystem::path texturePath = EMERGENCE_BUILD_STRING (selectedRoot, "/", assetId);
                FILE *file = std::fopen (texturePath.generic_string ().c_str (), "rb");

                if (!file)
                {
                    EMERGENCE_LOG (ERROR, "TextureManagement: Unable to open texture file \"",
                                   texturePath.generic_string<char, std::char_traits<char>, Memory::HeapSTD<char>> (),
                                   "\".");
                    sharedState->state = AssetState::CORRUPTED;
                    return;
                }

                fseek (file, 0u, SEEK_END);
                sharedState->textureDataSize = static_cast<uint64_t> (ftell (file));
                fseek (file, 0u, SEEK_SET);
                sharedState->textureData = static_cast<uint8_t *> (
                    sharedState->textureDataHeap.Acquire (sharedState->textureDataSize, alignof (uint8_t)));

                if (fread (sharedState->textureData, 1u, sharedState->textureDataSize, file) !=
                    sharedState->textureDataSize)
                {
                    EMERGENCE_LOG (ERROR, "TextureManagement: Unable to read texture file \"",
                                   texturePath.generic_string<char, std::char_traits<char>, Memory::HeapSTD<char>> (),
                                   "\".");
                    sharedState->state = AssetState::CORRUPTED;
                    return;
                }

                fclose (file);
                sharedState->state = AssetState::READY;
            });

        return AssetState::LOADING;
    }

    EMERGENCE_LOG (ERROR, "TextureManagement: Unable to find texture settings for texture \"", _loadingState->assetId,
                   "\".");
    return AssetState::MISSING;
}

AssetState Manager::TryFinishLoading (TextureLoadingState *_loadingState) noexcept
{
    Render::Backend::Texture nativeTexture {_loadingState->sharedState->textureData,
                                            _loadingState->sharedState->textureDataSize,
                                            _loadingState->sharedState->settings};
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

void Manager::Unload (Memory::UniqueString _assetId) noexcept
{
    if (auto textureCursor = removeTextureById.Execute (&_assetId); textureCursor.ReadConst ())
    {
        ~textureCursor;
    }
}

void AddToNormalUpdate (PipelineBuilder &_pipelineBuilder,
                        const Container::Vector<Memory::UniqueString> &_textureRootPaths,
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
        .SetExecutor<Manager> (_textureRootPaths, iterator->second);
}
} // namespace Emergence::Celerity::TextureManagement

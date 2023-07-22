#include <Celerity/Asset/Asset.hpp>
#include <Celerity/Asset/Events.hpp>
#include <Celerity/Asset/Render/Foundation/TextureLoadingState.hpp>
#include <Celerity/Asset/Render/Foundation/TextureManagement.hpp>
#include <Celerity/Asset/StatefulAssetManagerBase.hpp>
#include <Celerity/PipelineBuilderMacros.hpp>
#include <Celerity/Render/Foundation/Texture.hpp>

#include <Job/Dispatcher.hpp>

#include <Log/Log.hpp>

namespace Emergence::Celerity::TextureManagement
{
using namespace Memory::Literals;

class Manager : public TaskExecutorBase<Manager>, public StatefulAssetManagerBase<Manager>
{
public:
    using AssetType = Texture;

    using LoadingState = TextureLoadingState;

    Manager (TaskConstructor &_constructor,
             Resource::Provider::ResourceProvider *_resourceProvider,
             const StandardLayout::Mapping &_stateUpdateEvent) noexcept;

private:
    friend class StatefulAssetManagerBase<Manager>;

    AssetState StartLoading (TextureLoadingState *_loadingState) noexcept;

    AssetState TryFinishLoading (TextureLoadingState *_loadingState) noexcept;

    void Unload (Memory::UniqueString _assetId) noexcept;

    InsertLongTermQuery insertTexture;
    RemoveValueQuery removeTextureById;

    Resource::Provider::ResourceProvider *resourceProvider;
};

Manager::Manager (TaskConstructor &_constructor,
                  Resource::Provider::ResourceProvider *_resourceProvider,
                  const StandardLayout::Mapping &_stateUpdateEvent) noexcept
    : TaskExecutorBase (_constructor),
      StatefulAssetManagerBase<Manager> (_constructor, _stateUpdateEvent),

      insertTexture (INSERT_LONG_TERM (Texture)),
      removeTextureById (REMOVE_VALUE_1F (Texture, assetId)),

      resourceProvider (_resourceProvider)
{
}

AssetState Manager::StartLoading (TextureLoadingState *_loadingState) noexcept
{
    Job::Dispatcher::Global ().Dispatch (
        Job::Priority::BACKGROUND,
        [assetId {_loadingState->assetId}, cachedResourceProvider {resourceProvider},
         sharedState {_loadingState->sharedState}] ()
        {
            static CPU::Profiler::SectionDefinition loadingSection {*"TextureLoading"_us, 0xFF999900u};
            CPU::Profiler::SectionInstance section {loadingSection};

            switch (cachedResourceProvider->LoadObject (TextureAsset::Reflect ().mapping, assetId, &sharedState->asset))
            {
            case Resource::Provider::LoadingOperationResponse::SUCCESSFUL:
                break;

            case Resource::Provider::LoadingOperationResponse::NOT_FOUND:
                EMERGENCE_LOG (ERROR, "TextureManagement: Unable to find texture \"", assetId, "\".");
                sharedState->state = AssetState::MISSING;
                return;

            case Resource::Provider::LoadingOperationResponse::IO_ERROR:
                EMERGENCE_LOG (ERROR, "TextureManagement: Failed to read texture \"", assetId, "\".");
                sharedState->state = AssetState::CORRUPTED;
                return;

            case Resource::Provider::LoadingOperationResponse::WRONG_TYPE:
                EMERGENCE_LOG (ERROR, "TextureManagement: Object \"", assetId, "\" is not a texture.");
                sharedState->state = AssetState::CORRUPTED;
                return;
            }

            switch (cachedResourceProvider->LoadThirdPartyResource (
                sharedState->asset.textureId, sharedState->textureDataHeap, sharedState->textureDataSize,
                sharedState->textureData))
            {
            case Resource::Provider::LoadingOperationResponse::SUCCESSFUL:
                break;

            case Resource::Provider::LoadingOperationResponse::NOT_FOUND:
                EMERGENCE_LOG (ERROR, "TextureManagement: Unable to find texture source \"",
                               sharedState->asset.textureId, "\".");
                sharedState->state = AssetState::MISSING;
                return;

            case Resource::Provider::LoadingOperationResponse::IO_ERROR:
                EMERGENCE_LOG (ERROR, "TextureManagement: Failed to read texture source \"",
                               sharedState->asset.textureId, "\".");
                sharedState->state = AssetState::CORRUPTED;
                return;

            case Resource::Provider::LoadingOperationResponse::WRONG_TYPE:
                sharedState->state = AssetState::CORRUPTED;
                return;
            }

            sharedState->state = AssetState::READY;
        });

    return AssetState::LOADING;
}

AssetState Manager::TryFinishLoading (TextureLoadingState *_loadingState) noexcept
{
    if (_loadingState->sharedState->state != AssetState::READY)
    {
        return _loadingState->sharedState->state;
    }

    Render::Backend::Texture nativeTexture = Render::Backend::Texture::CreateFromFile (
        _loadingState->sharedState->textureData, _loadingState->sharedState->textureDataSize,
        _loadingState->sharedState->asset.settings);

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
                        Resource::Provider::ResourceProvider *_resourceProvider,
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
    _pipelineBuilder.AddTask ("TextureManager"_us).SetExecutor<Manager> (_resourceProvider, iterator->second);
}
} // namespace Emergence::Celerity::TextureManagement

#include <Celerity/Asset/AssetManagement.hpp>
#include <Celerity/Asset/Events.hpp>
#include <Celerity/Asset/Render/Foundation/Material.hpp>
#include <Celerity/Asset/Render/Foundation/MaterialLoadingState.hpp>
#include <Celerity/Asset/Render/Foundation/MaterialManagement.hpp>
#include <Celerity/Asset/StatefulAssetManagerBase.hpp>
#include <Celerity/PipelineBuilderMacros.hpp>

#include <Job/Dispatcher.hpp>

#include <Log/Log.hpp>

#include <Render/Backend/Configuration.hpp>

namespace Emergence::Celerity::MaterialManagement
{
class Manager final : public TaskExecutorBase<Manager>, public StatefulAssetManagerBase<Manager>
{
public:
    using AssetType = Material;

    using LoadingState = MaterialLoadingState;

    Manager (TaskConstructor &_constructor,
             Resource::Provider::ResourceProvider *_resourceProvider,
             const StandardLayout::Mapping &_stateUpdateEvent) noexcept;

private:
    friend class StatefulAssetManagerBase<Manager>;

    AssetState StartLoading (MaterialLoadingState *_loadingState) noexcept;

    AssetState TryFinishLoading (MaterialLoadingState *_loadingState) noexcept;

    bool RegisterUniform (Memory::UniqueString _assetId, const UniformDescription &_description) noexcept;

    void Unload (Memory::UniqueString _assetId) noexcept;

    InsertLongTermQuery insertMaterial;
    InsertLongTermQuery insertUniform;

    RemoveValueQuery removeMaterialById;
    RemoveValueQuery removeUniformById;

    Resource::Provider::ResourceProvider *resourceProvider;
};

Manager::Manager (TaskConstructor &_constructor,
                  Resource::Provider::ResourceProvider *_resourceProvider,
                  const StandardLayout::Mapping &_stateUpdateEvent) noexcept
    : StatefulAssetManagerBase (_constructor, _stateUpdateEvent),

      insertMaterial (INSERT_LONG_TERM (Material)),
      insertUniform (INSERT_LONG_TERM (Uniform)),

      removeMaterialById (REMOVE_VALUE_1F (Material, assetId)),
      removeUniformById (REMOVE_VALUE_1F (Uniform, assetId)),

      resourceProvider (_resourceProvider)
{
    _constructor.DependOn (AssetManagement::Checkpoint::ASSET_LOADING_STARTED);
    _constructor.MakeDependencyOf (AssetManagement::Checkpoint::ASSET_LOADING_FINISHED);
}

AssetState Manager::StartLoading (MaterialLoadingState *_loadingState) noexcept
{
    Job::Dispatcher::Global ().Dispatch (
        Job::Priority::BACKGROUND,
        [assetId {_loadingState->assetId}, cachedResourceProvider {resourceProvider},
         sharedState {_loadingState->sharedState}] ()
        {
            switch (
                cachedResourceProvider->LoadObject (MaterialAsset::Reflect ().mapping, assetId, &sharedState->asset))
            {
            case Resource::Provider::LoadingOperationResponse::SUCCESSFUL:
                break;

            case Resource::Provider::LoadingOperationResponse::NOT_FOUND:
                EMERGENCE_LOG (ERROR, "MaterialManagement: Unable to find material \"", assetId, "\".");
                sharedState->state = AssetState::MISSING;
                return;

            case Resource::Provider::LoadingOperationResponse::IO_ERROR:
                EMERGENCE_LOG (ERROR, "MaterialManagement: Failed to read material \"", assetId, "\".");
                sharedState->state = AssetState::CORRUPTED;
                return;

            case Resource::Provider::LoadingOperationResponse::WRONG_TYPE:
                EMERGENCE_LOG (ERROR, "MaterialManagement: Object \"", assetId, "\" is not a material.");
                sharedState->state = AssetState::CORRUPTED;
                return;
            }

            const Memory::UniqueString vertexShaderId {EMERGENCE_BUILD_STRING (
                sharedState->asset.vertexShader, ".vertex", Render::Backend::Program::GetShaderSuffix ())};

            switch (cachedResourceProvider->LoadThirdPartyResource (vertexShaderId, sharedState->shaderDataHeap,
                                                                    sharedState->vertexSharedSize,
                                                                    sharedState->vertexShaderData))
            {
            case Resource::Provider::LoadingOperationResponse::SUCCESSFUL:
                break;

            case Resource::Provider::LoadingOperationResponse::NOT_FOUND:
                EMERGENCE_LOG (ERROR, "MaterialManagement: Unable to find vertex shader \"", vertexShaderId, "\".");
                sharedState->state = AssetState::MISSING;
                return;

            case Resource::Provider::LoadingOperationResponse::IO_ERROR:
                EMERGENCE_LOG (ERROR, "MaterialManagement: Failed to read vertex shader \"", vertexShaderId, "\".");
                sharedState->state = AssetState::CORRUPTED;
                return;

            case Resource::Provider::LoadingOperationResponse::WRONG_TYPE:
                sharedState->state = AssetState::CORRUPTED;
                return;
            }

            const Memory::UniqueString fragmentShaderId {EMERGENCE_BUILD_STRING (
                sharedState->asset.fragmentShader, ".fragment", Render::Backend::Program::GetShaderSuffix ())};

            switch (cachedResourceProvider->LoadThirdPartyResource (fragmentShaderId, sharedState->shaderDataHeap,
                                                                    sharedState->fragmentSharedSize,
                                                                    sharedState->fragmentShaderData))
            {
            case Resource::Provider::LoadingOperationResponse::SUCCESSFUL:
                break;

            case Resource::Provider::LoadingOperationResponse::NOT_FOUND:
                EMERGENCE_LOG (ERROR, "MaterialManagement: Unable to find fragment shader \"", fragmentShaderId, "\".");
                sharedState->state = AssetState::MISSING;
                return;

            case Resource::Provider::LoadingOperationResponse::IO_ERROR:
                EMERGENCE_LOG (ERROR, "MaterialManagement: Failed to read fragment shader \"", fragmentShaderId, "\".");
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

AssetState Manager::TryFinishLoading (MaterialLoadingState *_loadingState) noexcept
{
    auto materialCursor = insertMaterial.Execute ();
    auto *material = static_cast<Material *> (++materialCursor);
    material->assetId = _loadingState->assetId;
    material->vertexShader = _loadingState->sharedState->asset.vertexShader;
    material->fragmentShader = _loadingState->sharedState->asset.fragmentShader;

    material->program = {_loadingState->sharedState->vertexShaderData, _loadingState->sharedState->vertexSharedSize,
                         _loadingState->sharedState->fragmentShaderData,
                         _loadingState->sharedState->fragmentSharedSize};

    if (!material->program.IsValid ())
    {
        EMERGENCE_LOG (ERROR, "MaterialManagement: Unable to create program for material \"", material->assetId, "\".");
        return AssetState::CORRUPTED;
    }

    for (const UniformDescription &description : _loadingState->sharedState->asset.uniforms)
    {
        if (!RegisterUniform (_loadingState->assetId, description))
        {
            return AssetState::CORRUPTED;
        }
    }

    return AssetState::READY;
}

bool Manager::RegisterUniform (Memory::UniqueString _assetId, const UniformDescription &_description) noexcept
{
    Render::Backend::Uniform nativeUniform {_description.name, _description.type};
    if (!nativeUniform.IsValid ())
    {
        EMERGENCE_LOG (ERROR, "MaterialManagement: Unable to register uniform \"", _description.name, "\".");
        return false;
    }

    auto cursor = insertUniform.Execute ();
    auto *uniform = static_cast<Uniform *> (++cursor);

    uniform->assetId = _assetId;
    uniform->name = _description.name;
    uniform->type = _description.type;
    uniform->uniform = std::move (nativeUniform);

    if (_description.type == Render::Backend::UniformType::SAMPLER)
    {
        uniform->textureStage = _description.textureStage;
    }

    return true;
}

void Manager::Unload (Memory::UniqueString _assetId) noexcept
{
    for (auto uniformCursor = removeUniformById.Execute (&_assetId); uniformCursor.ReadConst (); ~uniformCursor)
    {
    }

    if (auto materialCursor = removeMaterialById.Execute (&_assetId); materialCursor.ReadConst ())
    {
        ~materialCursor;
    }
}

void AddToNormalUpdate (PipelineBuilder &_pipelineBuilder,
                        Resource::Provider::ResourceProvider *_resourceProvider,
                        const AssetReferenceBindingEventMap &_eventMap) noexcept
{
    auto iterator = _eventMap.stateUpdate.find (Material::Reflect ().mapping);
    if (iterator == _eventMap.stateUpdate.end ())
    {
        EMERGENCE_LOG (WARNING,
                       "MaterialManagement: Task not registered, because Material is not found "
                       "in state update map. Perhaps it is not referenced by anything?");
        return;
    }

    auto visualGroup = _pipelineBuilder.OpenVisualGroup ("MaterialManagement");
    _pipelineBuilder.AddTask (Memory::UniqueString {"MaterialManager"})
        .SetExecutor<Manager> (_resourceProvider, iterator->second);
}
} // namespace Emergence::Celerity::MaterialManagement

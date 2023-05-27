#define _CRT_SECURE_NO_WARNINGS

#include <Celerity/Asset/AssetManagement.hpp>
#include <Celerity/Asset/Events.hpp>
#include <Celerity/Asset/Render/Foundation/MaterialInstance.hpp>
#include <Celerity/Asset/Render/Foundation/MaterialInstanceLoadingState.hpp>
#include <Celerity/Asset/Render/Foundation/MaterialInstanceManagement.hpp>
#include <Celerity/Asset/StatefulAssetManagerBase.hpp>
#include <Celerity/PipelineBuilderMacros.hpp>

#include <Container/Algorithm.hpp>

#include <Job/Dispatcher.hpp>

#include <Log/Log.hpp>

namespace Emergence::Celerity::MaterialInstanceManagement
{
class Manager : public TaskExecutorBase<Manager>, public StatefulAssetManagerBase<Manager>
{
public:
    using AssetType = MaterialInstance;

    using LoadingState = MaterialInstanceLoadingState;

    Manager (TaskConstructor &_constructor,
             Resource::Provider::ResourceProvider *_resourceProvider,
             const StandardLayout::Mapping &_stateUpdateEvent) noexcept;

private:
    friend class StatefulAssetManagerBase<Manager>;

    AssetState GetDependencyState (Memory::UniqueString _assetId) noexcept;

    AssetState StartLoading (MaterialInstanceLoadingState *_loadingState) noexcept;

    void CopyParentValuesIntoCollector (Memory::UniqueString _assetId) noexcept;

    AssetState TryFinishLoading (MaterialInstanceLoadingState *_loadingState) noexcept;

    void Unload (Memory::UniqueString _assetId) noexcept;

    InsertLongTermQuery insertMaterialInstance;
    InsertLongTermQuery insertUniformVector4fValue;
    InsertLongTermQuery insertUniformMatrix3x3fValue;
    InsertLongTermQuery insertUniformMatrix4x4fValue;
    InsertLongTermQuery insertUniformSamplerValue;

    FetchValueQuery fetchAssetById;
    FetchValueQuery fetchMaterialInstanceById;
    EditValueQuery editMaterialInstanceById;
    FetchValueQuery fetchUniformVector4fValueById;
    FetchValueQuery fetchUniformMatrix3x3fValueById;
    FetchValueQuery fetchUniformMatrix4x4fValueById;
    FetchValueQuery fetchUniformSamplerValueById;

    RemoveValueQuery removeMaterialInstanceById;
    RemoveValueQuery removeUniformVector4fValueById;
    RemoveValueQuery removeUniformMatrix3x3fValueById;
    RemoveValueQuery removeUniformMatrix4x4fValueById;
    RemoveValueQuery removeUniformSamplerValueById;

    Resource::Provider::ResourceProvider *resourceProvider;
    Container::Vector<UniformValueDescription> uniformValuesCollector {Memory::Profiler::AllocationGroup::Top ()};
};

Manager::Manager (TaskConstructor &_constructor,
                  Resource::Provider::ResourceProvider *_resourceProvider,
                  const StandardLayout::Mapping &_stateUpdateEvent) noexcept
    : StatefulAssetManagerBase<Manager> (_constructor, _stateUpdateEvent),

      insertMaterialInstance (INSERT_LONG_TERM (MaterialInstance)),
      insertUniformVector4fValue (INSERT_LONG_TERM (UniformVector4fValue)),
      insertUniformMatrix3x3fValue (INSERT_LONG_TERM (UniformMatrix3x3fValue)),
      insertUniformMatrix4x4fValue (INSERT_LONG_TERM (UniformMatrix4x4fValue)),
      insertUniformSamplerValue (INSERT_LONG_TERM (UniformSamplerValue)),

      fetchAssetById (FETCH_VALUE_1F (Asset, id)),
      fetchMaterialInstanceById (FETCH_VALUE_1F (MaterialInstance, assetId)),
      editMaterialInstanceById (EDIT_VALUE_1F (MaterialInstance, assetId)),
      fetchUniformVector4fValueById (FETCH_VALUE_1F (UniformVector4fValue, assetId)),
      fetchUniformMatrix3x3fValueById (FETCH_VALUE_1F (UniformMatrix3x3fValue, assetId)),
      fetchUniformMatrix4x4fValueById (FETCH_VALUE_1F (UniformMatrix4x4fValue, assetId)),
      fetchUniformSamplerValueById (FETCH_VALUE_1F (UniformSamplerValue, assetId)),

      removeMaterialInstanceById (REMOVE_VALUE_1F (MaterialInstance, assetId)),
      removeUniformVector4fValueById (REMOVE_VALUE_1F (UniformVector4fValue, assetId)),
      removeUniformMatrix3x3fValueById (REMOVE_VALUE_1F (UniformMatrix3x3fValue, assetId)),
      removeUniformMatrix4x4fValueById (REMOVE_VALUE_1F (UniformMatrix4x4fValue, assetId)),
      removeUniformSamplerValueById (REMOVE_VALUE_1F (UniformSamplerValue, assetId)),

      resourceProvider (_resourceProvider)
{
    _constructor.DependOn (AssetManagement::Checkpoint::ASSET_LOADING_STARTED);
    _constructor.MakeDependencyOf (AssetManagement::Checkpoint::ASSET_LOADING_FINISHED);
}

AssetState Manager::GetDependencyState (Memory::UniqueString _assetId) noexcept
{
    if (auto assetCursor = fetchAssetById.Execute (&_assetId);
        const auto *asset = static_cast<const Asset *> (*assetCursor))
    {
        return asset->state;
    }

    // If asset does not exist, it is not yet processed by manager,
    // therefore we're returning loading state as default values that is neither error nor ready flag.
    return AssetState::LOADING;
}

AssetState Manager::StartLoading (MaterialInstanceLoadingState *_loadingState) noexcept
{
    const char *runtimeIdSeparator = strchr (*_loadingState->assetId, MATERIAL_INSTANCE_RUNTIME_ID_SEPARATOR);
    if (runtimeIdSeparator)
    {
        _loadingState->sharedState->asset.parent = Memory::UniqueString {std::string_view (
            *_loadingState->assetId, static_cast<size_t> (runtimeIdSeparator - *_loadingState->assetId))};
        _loadingState->sharedState->state = AssetState::READY;
    }
    else
    {
        Job::Dispatcher::Global ().Dispatch (
            Job::Priority::BACKGROUND,
            [assetId {_loadingState->assetId}, cachedResourceProvider {resourceProvider},
             sharedState {_loadingState->sharedState}] ()
            {
                switch (cachedResourceProvider->LoadObject (MaterialInstanceAsset::Reflect ().mapping, assetId,
                                                            &sharedState->asset))
                {
                case Resource::Provider::LoadingOperationResponse::SUCCESSFUL:
                    sharedState->state = AssetState::READY;
                    break;

                case Resource::Provider::LoadingOperationResponse::NOT_FOUND:
                    EMERGENCE_LOG (ERROR, "MaterialInstanceManagement: Unable to find material instance \"", assetId,
                                   "\".");
                    sharedState->state = AssetState::MISSING;
                    break;

                case Resource::Provider::LoadingOperationResponse::IO_ERROR:
                    EMERGENCE_LOG (ERROR, "MaterialInstanceManagement: Failed to read material instance \"", assetId,
                                   "\".");
                    sharedState->state = AssetState::CORRUPTED;
                    break;

                case Resource::Provider::LoadingOperationResponse::WRONG_TYPE:
                    EMERGENCE_LOG (ERROR, "MaterialInstanceManagement: Object \"", assetId,
                                   "\" is not a material instance.");
                    sharedState->state = AssetState::CORRUPTED;
                    break;
                }
            });
    }

    return AssetState::LOADING;
}

AssetState Manager::TryFinishLoading (MaterialInstanceLoadingState *_loadingState) noexcept
{
    switch (_loadingState->sharedState->state)
    {
    case AssetState::LOADING:
        return AssetState::LOADING;

    case AssetState::MISSING:
        return AssetState::MISSING;

    case AssetState::CORRUPTED:
        return AssetState::CORRUPTED;

    case AssetState::READY:
        break;
    }

    _loadingState->parentId = _loadingState->sharedState->asset.parent;
    if (!*_loadingState->sharedState->asset.material && !*_loadingState->sharedState->asset.parent)
    {
        EMERGENCE_LOG (ERROR, "MaterialInstanceManager: Parentless material instance \"", _loadingState->assetId,
                       "\" is not attached to any material!");
        return AssetState::CORRUPTED;
    }

    const bool materialInstanceExists = *fetchMaterialInstanceById.Execute (&_loadingState->assetId);
    if (!materialInstanceExists)
    {
        auto materialInstanceInsertCursor = insertMaterialInstance.Execute ();
        auto *materialInstance = static_cast<MaterialInstance *> (++materialInstanceInsertCursor);
        materialInstance->assetId = _loadingState->assetId;
        materialInstance->materialId = _loadingState->sharedState->asset.material;
    }

    Memory::UniqueString materialId = _loadingState->sharedState->asset.material;
    if (*_loadingState->sharedState->asset.parent)
    {
        switch (GetDependencyState (_loadingState->sharedState->asset.parent))
        {
        case AssetState::LOADING:
            return AssetState::LOADING;

        case AssetState::MISSING:
            return AssetState::MISSING;

        case AssetState::CORRUPTED:
            return AssetState::CORRUPTED;

        case AssetState::READY:
            break;
        }

        if (!*materialId)
        {
            {
                auto parentMaterialInstanceCursor =
                    fetchMaterialInstanceById.Execute (&_loadingState->sharedState->asset.parent);
                const auto *parentMaterialInstance =
                    static_cast<const MaterialInstance *> (*parentMaterialInstanceCursor);
                materialId = parentMaterialInstance->materialId;
            }

            auto materialInstanceCursor = editMaterialInstanceById.Execute (&_loadingState->assetId);
            auto *materialInstance = static_cast<MaterialInstance *> (*materialInstanceCursor);
            materialInstance->materialId = materialId;
        }
#if defined(EMERGENCE_ASSERT_ENABLED)
        else
        {
            auto parentMaterialInstanceCursor =
                fetchMaterialInstanceById.Execute (&_loadingState->sharedState->asset.parent);
            const auto *parentMaterialInstance = static_cast<const MaterialInstance *> (*parentMaterialInstanceCursor);
            EMERGENCE_ASSERT (materialId == parentMaterialInstance->materialId);
        }
#endif
    }

    switch (GetDependencyState (materialId))
    {
    case AssetState::LOADING:
        return AssetState::LOADING;

    case AssetState::MISSING:
        return AssetState::MISSING;

    case AssetState::CORRUPTED:
        return AssetState::CORRUPTED;

    case AssetState::READY:
        break;
    }

    for (const UniformValueDescription &uniform : _loadingState->sharedState->asset.uniforms)
    {
        for (const UniformValueDescription &item : uniformValuesCollector)
        {
            if (item.name == uniform.name)
            {
                EMERGENCE_LOG (ERROR, "MaterialInstanceManagement: Material instance \"", _loadingState->assetId,
                               "\" has several values with name \"", item.name, "\".");
                return AssetState::CORRUPTED;
            }
        }

        uniformValuesCollector.emplace_back (uniform);
    }

    if (*_loadingState->sharedState->asset.parent)
    {
        CopyParentValuesIntoCollector (_loadingState->sharedState->asset.parent);
    }

    auto insertVector4Cursor = insertUniformVector4fValue.Execute ();
    auto insertMatrix3x3Cursor = insertUniformMatrix3x3fValue.Execute ();
    auto insertMatrix4x4Cursor = insertUniformMatrix4x4fValue.Execute ();
    auto insertSamplerCursor = insertUniformSamplerValue.Execute ();

    for (const UniformValueDescription &value : uniformValuesCollector)
    {
        switch (value.type)
        {
        case Render::Backend::UniformType::VECTOR_4F:
        {
            auto *uniformValue = static_cast<UniformVector4fValue *> (++insertVector4Cursor);
            uniformValue->assetId = _loadingState->assetId;
            uniformValue->uniformName = value.name;
            uniformValue->value = value.vector4f;
            break;
        }

        case Render::Backend::UniformType::MATRIX_3X3F:
        {
            auto *uniformValue = static_cast<UniformMatrix3x3fValue *> (++insertMatrix3x3Cursor);
            uniformValue->assetId = _loadingState->assetId;
            uniformValue->uniformName = value.name;
            uniformValue->value = value.matrix3x3f;
            break;
        }

        case Render::Backend::UniformType::MATRIX_4X4F:
        {
            auto *uniformValue = static_cast<UniformMatrix4x4fValue *> (++insertMatrix4x4Cursor);
            uniformValue->assetId = _loadingState->assetId;
            uniformValue->uniformName = value.name;
            uniformValue->value = value.matrix4x4f;
            break;
        }

        case Render::Backend::UniformType::SAMPLER:
        {
            auto *uniformValue = static_cast<UniformSamplerValue *> (++insertSamplerCursor);
            uniformValue->assetId = _loadingState->assetId;
            uniformValue->uniformName = value.name;
            uniformValue->textureId = value.textureId;
            break;
        }
        }
    }

    uniformValuesCollector.clear ();
    return AssetState::READY;
}

void Manager::CopyParentValuesIntoCollector (Memory::UniqueString _assetId) noexcept
{
    auto findByNameLambda = [] (const Memory::UniqueString _name)
    {
        return [_name] (const UniformValueDescription &_item)
        {
            return _item.name == _name;
        };
    };

    for (auto uniformCursor = fetchUniformVector4fValueById.Execute (&_assetId);
         const auto *uniform = static_cast<const UniformVector4fValue *> (*uniformCursor); ++uniformCursor)
    {
        if (Container::FindIf (uniformValuesCollector.begin (), uniformValuesCollector.end (),
                               findByNameLambda (uniform->uniformName)) == uniformValuesCollector.end ())
        {
            uniformValuesCollector.emplace_back (uniform->uniformName, uniform->value);
        }
    }

    for (auto uniformCursor = fetchUniformMatrix3x3fValueById.Execute (&_assetId);
         const auto *uniform = static_cast<const UniformMatrix3x3fValue *> (*uniformCursor); ++uniformCursor)
    {
        if (Container::FindIf (uniformValuesCollector.begin (), uniformValuesCollector.end (),
                               findByNameLambda (uniform->uniformName)) == uniformValuesCollector.end ())
        {
            uniformValuesCollector.emplace_back (uniform->uniformName, uniform->value);
        }
    }

    for (auto uniformCursor = fetchUniformMatrix4x4fValueById.Execute (&_assetId);
         const auto *uniform = static_cast<const UniformMatrix4x4fValue *> (*uniformCursor); ++uniformCursor)
    {
        if (Container::FindIf (uniformValuesCollector.begin (), uniformValuesCollector.end (),
                               findByNameLambda (uniform->uniformName)) == uniformValuesCollector.end ())
        {
            uniformValuesCollector.emplace_back (uniform->uniformName, uniform->value);
        }
    }

    for (auto uniformCursor = fetchUniformSamplerValueById.Execute (&_assetId);
         const auto *uniform = static_cast<const UniformSamplerValue *> (*uniformCursor); ++uniformCursor)
    {
        if (Container::FindIf (uniformValuesCollector.begin (), uniformValuesCollector.end (),
                               findByNameLambda (uniform->uniformName)) == uniformValuesCollector.end ())
        {
            uniformValuesCollector.emplace_back (uniform->uniformName, uniform->textureId);
        }
    }
}

void Manager::Unload (Memory::UniqueString _assetId) noexcept
{
    if (auto materialInstanceCursor = removeMaterialInstanceById.Execute (&_assetId);
        materialInstanceCursor.ReadConst ())
    {
        ~materialInstanceCursor;
    }

    for (auto uniformValueCursor = removeUniformVector4fValueById.Execute (&_assetId); uniformValueCursor.ReadConst ();
         ~uniformValueCursor)
    {
    }

    for (auto uniformValueCursor = removeUniformMatrix3x3fValueById.Execute (&_assetId);
         uniformValueCursor.ReadConst (); ~uniformValueCursor)
    {
    }

    for (auto uniformValueCursor = removeUniformMatrix4x4fValueById.Execute (&_assetId);
         uniformValueCursor.ReadConst (); ~uniformValueCursor)
    {
    }

    for (auto uniformValueCursor = removeUniformSamplerValueById.Execute (&_assetId); uniformValueCursor.ReadConst ();
         ~uniformValueCursor)
    {
    }
}

void AddToNormalUpdate (PipelineBuilder &_pipelineBuilder,
                        Resource::Provider::ResourceProvider *_resourceProvider,
                        const AssetReferenceBindingEventMap &_eventMap) noexcept
{
    auto iterator = _eventMap.stateUpdate.find (MaterialInstance::Reflect ().mapping);
    if (iterator == _eventMap.stateUpdate.end ())
    {
        EMERGENCE_LOG (WARNING,
                       "MaterialInstanceManagement: Task not registered, because MaterialInstance  "
                       "is not found in state update map. Perhaps it is not referenced by anything?");
        return;
    }

    auto visualGroup = _pipelineBuilder.OpenVisualGroup ("MaterialInstanceManagement");
    _pipelineBuilder.AddTask (Memory::UniqueString {"MaterialInstanceManager"})
        .SetExecutor<Manager> (_resourceProvider, iterator->second);
}
} // namespace Emergence::Celerity::MaterialInstanceManagement

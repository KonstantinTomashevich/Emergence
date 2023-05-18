#include <filesystem>
#include <fstream>

#include <Celerity/Asset/AssetManagement.hpp>
#include <Celerity/Asset/Events.hpp>
#include <Celerity/Asset/Render/Foundation/MaterialInstance.hpp>
#include <Celerity/Asset/Render/Foundation/MaterialInstanceLoadingState.hpp>
#include <Celerity/Asset/Render/Foundation/MaterialInstanceManagement.hpp>
#include <Celerity/Asset/StatefulAssetManagerBase.hpp>
#include <Celerity/PipelineBuilderMacros.hpp>

#include <Job/Dispatcher.hpp>

#include <Log/Log.hpp>

#include <Serialization/Binary.hpp>
#include <Serialization/Yaml.hpp>

namespace Emergence::Celerity::MaterialInstanceManagement
{
class Manager : public TaskExecutorBase<Manager>, public StatefulAssetManagerBase<Manager>
{
public:
    using AssetType = MaterialInstance;

    using LoadingState = MaterialInstanceLoadingState;

    Manager (TaskConstructor &_constructor,
             const Container::Vector<Memory::UniqueString> &_materialInstanceRootPaths,
             const StandardLayout::Mapping &_stateUpdateEvent) noexcept;

private:
    friend class StatefulAssetManagerBase<Manager>;

    AssetState GetDependencyState (Memory::UniqueString _assetId) noexcept;

    AssetState SummarizeDependencyState (Memory::UniqueString _materialId, Memory::UniqueString _parentId) noexcept;

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
    FetchValueQuery fetchUniformVector4fValueById;
    FetchValueQuery fetchUniformMatrix3x3fValueById;
    FetchValueQuery fetchUniformMatrix4x4fValueById;
    FetchValueQuery fetchUniformSamplerValueById;

    RemoveValueQuery removeMaterialInstanceById;
    RemoveValueQuery removeUniformVector4fValueById;
    RemoveValueQuery removeUniformMatrix3x3fValueById;
    RemoveValueQuery removeUniformMatrix4x4fValueById;
    RemoveValueQuery removeUniformSamplerValueById;

    Container::Vector<Memory::UniqueString> materialInstanceRoots {Memory::Profiler::AllocationGroup::Top ()};
    Container::Vector<UniformValueDescription> uniformValuesCollector {Memory::Profiler::AllocationGroup::Top ()};
};

Manager::Manager (TaskConstructor &_constructor,
                  const Container::Vector<Emergence::Memory::UniqueString> &_materialInstanceRootPaths,
                  const StandardLayout::Mapping &_stateUpdateEvent) noexcept
    : StatefulAssetManagerBase<Manager> (_constructor, _stateUpdateEvent),

      insertMaterialInstance (INSERT_LONG_TERM (MaterialInstance)),
      insertUniformVector4fValue (INSERT_LONG_TERM (UniformVector4fValue)),
      insertUniformMatrix3x3fValue (INSERT_LONG_TERM (UniformMatrix3x3fValue)),
      insertUniformMatrix4x4fValue (INSERT_LONG_TERM (UniformMatrix4x4fValue)),
      insertUniformSamplerValue (INSERT_LONG_TERM (UniformSamplerValue)),

      fetchAssetById (FETCH_VALUE_1F (Asset, id)),
      fetchMaterialInstanceById (FETCH_VALUE_1F (MaterialInstance, assetId)),
      fetchUniformVector4fValueById (FETCH_VALUE_1F (UniformVector4fValue, assetId)),
      fetchUniformMatrix3x3fValueById (FETCH_VALUE_1F (UniformMatrix3x3fValue, assetId)),
      fetchUniformMatrix4x4fValueById (FETCH_VALUE_1F (UniformMatrix4x4fValue, assetId)),
      fetchUniformSamplerValueById (FETCH_VALUE_1F (UniformSamplerValue, assetId)),

      removeMaterialInstanceById (REMOVE_VALUE_1F (MaterialInstance, assetId)),
      removeUniformVector4fValueById (REMOVE_VALUE_1F (UniformVector4fValue, assetId)),
      removeUniformMatrix3x3fValueById (REMOVE_VALUE_1F (UniformMatrix3x3fValue, assetId)),
      removeUniformMatrix4x4fValueById (REMOVE_VALUE_1F (UniformMatrix4x4fValue, assetId)),
      removeUniformSamplerValueById (REMOVE_VALUE_1F (UniformSamplerValue, assetId))
{
    materialInstanceRoots.reserve (_materialInstanceRootPaths.size ());
    for (Memory::UniqueString root : _materialInstanceRootPaths)
    {
        materialInstanceRoots.emplace_back (root);
    }

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

AssetState Manager::SummarizeDependencyState (Memory::UniqueString _materialId, Memory::UniqueString _parentId) noexcept
{
    const AssetState materialState = GetDependencyState (_materialId);
    bool allDependenciesReady = materialState == AssetState::READY;
    bool anyDependencyErrored = materialState == AssetState::MISSING || materialState == AssetState::CORRUPTED;

    if (*_parentId)
    {
        const AssetState parentState = GetDependencyState (_parentId);
        allDependenciesReady &= parentState == AssetState::READY;
        anyDependencyErrored |= parentState == AssetState::MISSING || parentState == AssetState::CORRUPTED;
    }

    if (anyDependencyErrored)
    {
        return AssetState::CORRUPTED;
    }

    if (allDependenciesReady)
    {
        return AssetState::READY;
    }

    return AssetState::LOADING;
}

AssetState Manager::StartLoading (MaterialInstanceLoadingState *_loadingState) noexcept
{
    for (Memory::UniqueString root : materialInstanceRoots)
    {
        std::filesystem::path binaryPath =
            EMERGENCE_BUILD_STRING (root, "/", _loadingState->assetId, ".material.instance.bin");
        if (std::filesystem::exists (binaryPath))
        {
            Job::Dispatcher::Global ().Dispatch (
                Job::Priority::BACKGROUND,
                [sharedState {_loadingState->sharedState}, binaryPath] ()
                {
                    std::ifstream input {binaryPath, std::ios::binary};
                    if (!Serialization::Binary::DeserializeObject (input, &sharedState->asset,
                                                                   MaterialInstanceAsset::Reflect ().mapping, {}))
                    {
                        EMERGENCE_LOG (
                            ERROR, "MaterialInstanceManagement: Unable to load material instance from \"",
                            binaryPath.generic_string<char, std::char_traits<char>, Memory::HeapSTD<char>> (), "\".");
                        sharedState->state = AssetState::CORRUPTED;
                    }
                    else
                    {
                        sharedState->state = AssetState::READY;
                    }
                });

            return AssetState::LOADING;
        }

        std::filesystem::path yamlPath =
            EMERGENCE_BUILD_STRING (root, "/", _loadingState->assetId, ".material.instance.yaml");
        if (std::filesystem::exists (yamlPath))
        {
            Job::Dispatcher::Global ().Dispatch (
                Job::Priority::BACKGROUND,
                [sharedState {_loadingState->sharedState}, yamlPath] ()
                {
                    std::ifstream input (yamlPath);
                    if (!Serialization::Yaml::DeserializeObject (input, &sharedState->asset,
                                                                 MaterialInstanceAsset::Reflect ().mapping, {}))
                    {
                        EMERGENCE_LOG (ERROR, "MaterialInstanceManagement: Unable to load material instance from \"",
                                       yamlPath.generic_string<char, std::char_traits<char>, Memory::HeapSTD<char>> (),
                                       "\".");
                        sharedState->state = AssetState::CORRUPTED;
                    }
                    else
                    {
                        sharedState->state = AssetState::READY;
                    }
                });

            return AssetState::LOADING;
        }
    }

    EMERGENCE_LOG (ERROR, "MaterialInstanceManagement: Unable to find material instance \"", _loadingState->assetId,
                   "\".");
    return AssetState::MISSING;
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
    if (!*_loadingState->sharedState->asset.material)
    {
        EMERGENCE_LOG (ERROR, "MaterialInstanceManager: Material instance \"", _loadingState->assetId,
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

    const AssetState dependencyState =
        SummarizeDependencyState (_loadingState->sharedState->asset.material, _loadingState->sharedState->asset.parent);

    if (dependencyState != AssetState::READY)
    {
        return dependencyState;
    }

#ifdef EMERGENCE_ASSERT_ENABLED
    if (*_loadingState->sharedState->asset.parent)
    {
        auto parentMaterialInstanceCursor =
            fetchMaterialInstanceById.Execute (&_loadingState->sharedState->asset.parent);
        const auto *parentMaterialInstance = static_cast<const MaterialInstance *> (*parentMaterialInstanceCursor);
        EMERGENCE_ASSERT (parentMaterialInstance);
        EMERGENCE_ASSERT (parentMaterialInstance->materialId == _loadingState->sharedState->asset.material);
    }
#endif

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
        if (std::find_if (uniformValuesCollector.begin (), uniformValuesCollector.end (),
                          findByNameLambda (uniform->uniformName)) == uniformValuesCollector.end ())
        {
            uniformValuesCollector.emplace_back (uniform->uniformName, uniform->value);
        }
    }

    for (auto uniformCursor = fetchUniformMatrix3x3fValueById.Execute (&_assetId);
         const auto *uniform = static_cast<const UniformMatrix3x3fValue *> (*uniformCursor); ++uniformCursor)
    {
        if (std::find_if (uniformValuesCollector.begin (), uniformValuesCollector.end (),
                          findByNameLambda (uniform->uniformName)) == uniformValuesCollector.end ())
        {
            uniformValuesCollector.emplace_back (uniform->uniformName, uniform->value);
        }
    }

    for (auto uniformCursor = fetchUniformMatrix4x4fValueById.Execute (&_assetId);
         const auto *uniform = static_cast<const UniformMatrix4x4fValue *> (*uniformCursor); ++uniformCursor)
    {
        if (std::find_if (uniformValuesCollector.begin (), uniformValuesCollector.end (),
                          findByNameLambda (uniform->uniformName)) == uniformValuesCollector.end ())
        {
            uniformValuesCollector.emplace_back (uniform->uniformName, uniform->value);
        }
    }

    for (auto uniformCursor = fetchUniformSamplerValueById.Execute (&_assetId);
         const auto *uniform = static_cast<const UniformSamplerValue *> (*uniformCursor); ++uniformCursor)
    {
        if (std::find_if (uniformValuesCollector.begin (), uniformValuesCollector.end (),
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
                        const Container::Vector<Memory::UniqueString> &_materialInstanceRootPaths,
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
        .SetExecutor<Manager> (_materialInstanceRootPaths, iterator->second);
}
} // namespace Emergence::Celerity::MaterialInstanceManagement

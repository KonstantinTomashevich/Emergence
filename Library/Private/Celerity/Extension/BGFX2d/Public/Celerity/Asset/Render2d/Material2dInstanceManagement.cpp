#include <filesystem>
#include <fstream>

#include <Celerity/Asset/AssetManagement.hpp>
#include <Celerity/Asset/AssetManagerSingleton.hpp>
#include <Celerity/Asset/Events.hpp>
#include <Celerity/Asset/Render2d/Material2dInstance.hpp>
#include <Celerity/Asset/Render2d/Material2dInstanceLoadingState.hpp>
#include <Celerity/Asset/Render2d/Material2dInstanceManagement.hpp>
#include <Celerity/PipelineBuilderMacros.hpp>

#include <Log/Log.hpp>

#include <Serialization/Binary.hpp>
#include <Serialization/Yaml.hpp>

#include <SyntaxSugar/Time.hpp>

namespace Emergence::Celerity::Material2dInstanceManagement
{
class Manager : public TaskExecutorBase<Manager>
{
public:
    Manager (TaskConstructor &_constructor,
             const Container::Vector<Memory::UniqueString> &_materialInstanceRootPaths,
             uint64_t _maxLoadingTimePerFrameNs,
             const StandardLayout::Mapping &_stateUpdateEvent) noexcept;

    void Execute () noexcept;

private:
    void ProcessLoading () noexcept;

    AssetState GetDependencyState (Memory::UniqueString _assetId) noexcept;

    AssetState SummarizeDependencyState (Memory::UniqueString _materialId, Memory::UniqueString _parentId) noexcept;

    AssetState TryInitializeLoading (Memory::UniqueString _assetId) noexcept;

    AssetState LoadInstanceValuesIntoCollector (Memory::UniqueString _assetId) noexcept;

    void CopyParentValuesIntoCollector (Memory::UniqueString _assetId) noexcept;

    AssetState TryFinalizeLoading (Memory::UniqueString _assetId, Memory::UniqueString _parentAssetId) noexcept;

    void ProcessUnloading () noexcept;

    void Unload (Memory::UniqueString _assetId) noexcept;

    FetchSingletonQuery fetchAssetManager;

    InsertShortTermQuery insertAssetStateEvent;
    FetchSequenceQuery fetchAssetRemovedEvents;
    FetchValueQuery fetchAssetById;
    FetchValueQuery fetchAssetByTypeNumberAndState;

    InsertLongTermQuery insertMaterialInstanceLoadingState;
    InsertLongTermQuery insertMaterialInstance;
    InsertLongTermQuery insertUniformVector4fValue;
    InsertLongTermQuery insertUniformMatrix3x3fValue;
    InsertLongTermQuery insertUniformMatrix4x4fValue;
    InsertLongTermQuery insertUniformSamplerValue;

    FetchValueQuery fetchMaterialInstanceById;
    FetchValueQuery fetchUniformVector4fValueById;
    FetchValueQuery fetchUniformMatrix3x3fValueById;
    FetchValueQuery fetchUniformMatrix4x4fValueById;
    FetchValueQuery fetchUniformSamplerValueById;

    RemoveValueQuery removeMaterialInstanceLoadingStateById;
    RemoveValueQuery removeMaterialInstanceById;
    RemoveValueQuery removeUniformVector4fValueById;
    RemoveValueQuery removeUniformMatrix3x3fValueById;
    RemoveValueQuery removeUniformMatrix4x4fValueById;
    RemoveValueQuery removeUniformSamplerValueById;

    Container::Vector<Memory::UniqueString> materialInstanceRoots {Memory::Profiler::AllocationGroup::Top ()};
    const uint64_t maxLoadingTimePerFrameNs;
    Container::Vector<UniformValueBundleItem> uniformValuesCollector {Memory::Profiler::AllocationGroup::Top ()};

    Serialization::FieldNameLookupCache instanceHeaderLookupCache {Material2dInstanceAssetHeader::Reflect ().mapping};
    Serialization::Yaml::ObjectBundleDeserializer uniformValueBundleDeserializer {
        UniformValueBundleItem::Reflect ().mapping};
};

Manager::Manager (TaskConstructor &_constructor,
                  const Container::Vector<Emergence::Memory::UniqueString> &_materialInstanceRootPaths,
                  uint64_t _maxLoadingTimePerFrameNs,
                  const StandardLayout::Mapping &_stateUpdateEvent) noexcept
    : fetchAssetManager (FETCH_SINGLETON (AssetManagerSingleton)),

      insertAssetStateEvent (_constructor.InsertShortTerm (_stateUpdateEvent)),
      fetchAssetRemovedEvents (FETCH_SEQUENCE (AssetRemovedNormalEvent)),
      fetchAssetById (FETCH_VALUE_1F (Asset, id)),
      fetchAssetByTypeNumberAndState (FETCH_VALUE_2F (Asset, typeNumber, state)),

      insertMaterialInstanceLoadingState (INSERT_LONG_TERM (Material2dInstanceLoadingState)),
      insertMaterialInstance (INSERT_LONG_TERM (Material2dInstance)),
      insertUniformVector4fValue (INSERT_LONG_TERM (UniformVector4fValue)),
      insertUniformMatrix3x3fValue (INSERT_LONG_TERM (UniformMatrix3x3fValue)),
      insertUniformMatrix4x4fValue (INSERT_LONG_TERM (UniformMatrix4x4fValue)),
      insertUniformSamplerValue (INSERT_LONG_TERM (UniformSamplerValue)),

      fetchMaterialInstanceById (FETCH_VALUE_1F (Material2dInstance, assetId)),
      fetchUniformVector4fValueById (FETCH_VALUE_1F (UniformVector4fValue, assetId)),
      fetchUniformMatrix3x3fValueById (FETCH_VALUE_1F (UniformMatrix3x3fValue, assetId)),
      fetchUniformMatrix4x4fValueById (FETCH_VALUE_1F (UniformMatrix4x4fValue, assetId)),
      fetchUniformSamplerValueById (FETCH_VALUE_1F (UniformSamplerValue, assetId)),

      removeMaterialInstanceLoadingStateById (REMOVE_VALUE_1F (Material2dInstanceLoadingState, assetId)),
      removeMaterialInstanceById (REMOVE_VALUE_1F (Material2dInstance, assetId)),
      removeUniformVector4fValueById (REMOVE_VALUE_1F (UniformVector4fValue, assetId)),
      removeUniformMatrix3x3fValueById (REMOVE_VALUE_1F (UniformMatrix3x3fValue, assetId)),
      removeUniformMatrix4x4fValueById (REMOVE_VALUE_1F (UniformMatrix4x4fValue, assetId)),
      removeUniformSamplerValueById (REMOVE_VALUE_1F (UniformSamplerValue, assetId)),

      maxLoadingTimePerFrameNs (_maxLoadingTimePerFrameNs)
{
    materialInstanceRoots.reserve (_materialInstanceRootPaths.size ());
    for (Memory::UniqueString root : _materialInstanceRootPaths)
    {
        materialInstanceRoots.emplace_back (root);
    }

    _constructor.DependOn (AssetManagement::Checkpoint::ASSET_LOADING_STARTED);
    _constructor.MakeDependencyOf (AssetManagement::Checkpoint::ASSET_LOADING_FINISHED);
}

void Manager::Execute () noexcept
{
    ProcessLoading ();
    ProcessUnloading ();
}

void Manager::ProcessLoading () noexcept
{
    struct
    {
        StandardLayout::Mapping mapping = Material2dInstance::Reflect ().mapping;
        AssetState state = AssetState::LOADING;
    } loadingMaterialsParameter;

    const uint64_t startTime = Emergence::Time::NanosecondsSinceStartup ();
    for (auto assetCursor = fetchAssetByTypeNumberAndState.Execute (&loadingMaterialsParameter);
         const auto *asset = static_cast<const Asset *> (*assetCursor); ++assetCursor)
    {
        if (Emergence::Time::NanosecondsSinceStartup () - startTime > maxLoadingTimePerFrameNs)
        {
            break;
        }

        AssetState newState = AssetState::LOADING;
        bool needsInitialization = true;

        if (auto stateCursor = removeMaterialInstanceLoadingStateById.Execute (&asset->id);
            const auto *loadingState = static_cast<const Material2dInstanceLoadingState *> (stateCursor.ReadConst ()))
        {
            needsInitialization = false;
            newState = TryFinalizeLoading (asset->id, loadingState->parentId);

            if (newState != AssetState::LOADING)
            {
                ~stateCursor;
            }
        }

        if (needsInitialization)
        {
            newState = TryInitializeLoading (asset->id);
        }

        if (newState != AssetState::LOADING)
        {
            auto eventCursor = insertAssetStateEvent.Execute ();
            auto *event = static_cast<AssetStateUpdateEventView *> (++eventCursor);
            event->assetId = asset->id;
            event->state = newState;
        }
    }
}

AssetState Manager::GetDependencyState (Memory::UniqueString _assetId) noexcept
{
    if (auto assetCursor = fetchAssetById.Execute (&_assetId);
        const auto *asset = static_cast<const Asset *> (*assetCursor))
    {
        return asset->state;
    }

    // If asset not exist, it is not yet processed by manager,
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

AssetState Manager::TryInitializeLoading (Memory::UniqueString _assetId) noexcept
{
    auto assetManagerCursor = fetchAssetManager.Execute ();
    const auto *assetManager = static_cast<const AssetManagerSingleton *> (*assetManagerCursor);

    Unload (_assetId);
    Material2dInstanceAssetHeader header;
    bool headerFound = false;

    for (Memory::UniqueString root : materialInstanceRoots)
    {
        std::filesystem::path binaryPath = EMERGENCE_BUILD_STRING (root, "/", _assetId, ".material.instance.bin");
        if (std::filesystem::exists (binaryPath))
        {
            headerFound = true;
            std::ifstream input {binaryPath, std::ios::binary};

            if (!Serialization::Binary::DeserializeObject (input, &header,
                                                           Material2dInstanceAssetHeader::Reflect ().mapping))
            {
                EMERGENCE_LOG (ERROR, "Material2dInstanceManagement: Unable to load header from \"",
                               binaryPath.generic_string<char, std::char_traits<char>, Memory::HeapSTD<char>> (),
                               "\".");
                return AssetState::CORRUPTED;
            }
        }

        std::filesystem::path yamlPath = EMERGENCE_BUILD_STRING (root, "/", _assetId, ".material.instance.yaml");
        if (std::filesystem::exists (yamlPath))
        {
            headerFound = true;
            std::ifstream input {yamlPath};

            if (!Serialization::Yaml::DeserializeObject (input, &header, instanceHeaderLookupCache))
            {
                EMERGENCE_LOG (ERROR, "Material2dInstanceManagement: Unable to load header from \"",
                               yamlPath.generic_string<char, std::char_traits<char>, Memory::HeapSTD<char>> (), "\".");
                return AssetState::CORRUPTED;
            }
        }
    }

    if (!headerFound)
    {
        return AssetState::MISSING;
    }

    if (!*header.material)
    {
        EMERGENCE_LOG (ERROR, "Material2dInstanceManager: Material instance \"", _assetId,
                       "\" is not attached to any material!");
        return AssetState::CORRUPTED;
    }

    auto materialInstanceInsertCursor = insertMaterialInstance.Execute ();
    auto *materialInstance = static_cast<Material2dInstance *> (++materialInstanceInsertCursor);

    materialInstance->assetId = _assetId;
    materialInstance->assetUserId = assetManager->GenerateAssetUserId ();
    materialInstance->materialId = header.material;

    const AssetState dependencyState = SummarizeDependencyState (header.material, header.parent);
    if (dependencyState == AssetState::CORRUPTED || dependencyState == AssetState::MISSING)
    {
        return AssetState::CORRUPTED;
    }

    if (dependencyState == AssetState::READY)
    {
        AssetState newState = TryFinalizeLoading (_assetId, header.parent);
        // All dependencies are ready, we cannot be still in loading.
        EMERGENCE_ASSERT (newState != AssetState::LOADING);
        return newState;
    }

    auto loadingStateInsertCursor = insertMaterialInstanceLoadingState.Execute ();
    auto *loadingState = static_cast<Material2dInstanceLoadingState *> (++loadingStateInsertCursor);

    loadingState->assetId = _assetId;
    loadingState->assetUserId = assetManager->GenerateAssetUserId ();
    loadingState->parentId = header.parent;
    return AssetState::LOADING;
}

AssetState Manager::TryFinalizeLoading (Memory::UniqueString _assetId, Memory::UniqueString _parentAssetId) noexcept
{
    auto materialInstanceCursor = fetchMaterialInstanceById.Execute (&_assetId);
    const auto *materialInstance = static_cast<const Material2dInstance *> (*materialInstanceCursor);
    EMERGENCE_ASSERT (materialInstance);

    const AssetState dependencyState = SummarizeDependencyState (materialInstance->materialId, _parentAssetId);
    if (dependencyState != AssetState::READY)
    {
        return dependencyState;
    }

#ifdef EMERGENCE_ASSERT
    if (*_parentAssetId)
    {
        auto parentMaterialInstanceCursor = fetchMaterialInstanceById.Execute (&_parentAssetId);
        const auto *parentMaterialInstance = static_cast<const Material2dInstance *> (*parentMaterialInstanceCursor);
        EMERGENCE_ASSERT (parentMaterialInstance);
        EMERGENCE_ASSERT (parentMaterialInstance->materialId == materialInstance->materialId);
    }
#endif

    const AssetState instanceValuesState = LoadInstanceValuesIntoCollector (_assetId);
    if (instanceValuesState != AssetState::READY)
    {
        return dependencyState;
    }

    if (*_parentAssetId)
    {
        CopyParentValuesIntoCollector (_parentAssetId);
    }

    auto assetManagerCursor = fetchAssetManager.Execute ();
    const auto *assetManager = static_cast<const AssetManagerSingleton *> (*assetManagerCursor);

    auto insertVector4Cursor = insertUniformVector4fValue.Execute ();
    auto insertMatrix3x3Cursor = insertUniformMatrix3x3fValue.Execute ();
    auto insertMatrix4x4Cursor = insertUniformMatrix4x4fValue.Execute ();
    auto insertSamplerCursor = insertUniformSamplerValue.Execute ();

    for (const UniformValueBundleItem &value : uniformValuesCollector)
    {
        switch (value.type)
        {
        case Uniform2dType::VECTOR_4F:
        {
            auto *uniformValue = static_cast<UniformVector4fValue *> (++insertVector4Cursor);
            uniformValue->assetId = _assetId;
            uniformValue->uniformName = value.name;
            uniformValue->value = value.vector4f;
            break;
        }

        case Uniform2dType::MATRIX_3X3F:
        {
            auto *uniformValue = static_cast<UniformMatrix3x3fValue *> (++insertMatrix3x3Cursor);
            uniformValue->assetId = _assetId;
            uniformValue->uniformName = value.name;
            uniformValue->value = value.matrix3x3f;
            break;
        }

        case Uniform2dType::MATRIX_4X4F:
        {
            auto *uniformValue = static_cast<UniformMatrix4x4fValue *> (++insertMatrix4x4Cursor);
            uniformValue->assetId = _assetId;
            uniformValue->uniformName = value.name;
            uniformValue->value = value.matrix4x4f;
            break;
        }

        case Uniform2dType::SAMPLER:
        {
            auto *uniformValue = static_cast<UniformSamplerValue *> (++insertSamplerCursor);
            uniformValue->assetId = _assetId;
            uniformValue->assetUserId = assetManager->GenerateAssetUserId ();
            uniformValue->uniformName = value.name;
            uniformValue->textureId = value.textureId;
            break;
        }
        }
    }

    uniformValuesCollector.clear ();
    return AssetState::READY;
}

AssetState Manager::LoadInstanceValuesIntoCollector (Memory::UniqueString _assetId) noexcept
{
    UniformValueBundleItem uniformItem;
    auto insertDeserializedItem = [this, &uniformItem, _assetId] ()
    {
        for (const UniformValueBundleItem &item : uniformValuesCollector)
        {
            if (item.name == uniformItem.name)
            {
                EMERGENCE_LOG (ERROR, "Material2dInstanceManagement: Material instance \"", _assetId,
                               "\" has several values with name \"", item.name, "\".");
                return false;
            }
        }

        uniformValuesCollector.emplace_back (std::move (uniformItem));
        return true;
    };

    for (Memory::UniqueString root : materialInstanceRoots)
    {
        std::filesystem::path binaryUniformsPath = EMERGENCE_BUILD_STRING (root, "/", _assetId, ".uniform.values.bin");
        if (std::filesystem::exists (binaryUniformsPath))
        {
            std::ifstream input {binaryUniformsPath, std::ios::binary};
            // We need to do get-unget in order to force empty file check. Otherwise, it is not guaranteed.
            input.get ();
            input.unget ();

            while (input)
            {
                if (!Serialization::Binary::DeserializeObject (input, &uniformItem,
                                                               UniformValueBundleItem::Reflect ().mapping))
                {
                    EMERGENCE_LOG (
                        ERROR, "Material2dInstanceManagement: Unable to deserialize uniform values \"",
                        binaryUniformsPath.generic_string<char, std::char_traits<char>, Memory::HeapSTD<char>> (),
                        "\".");
                    return AssetState::CORRUPTED;
                }

                insertDeserializedItem ();
                // Use peek to test for the end of file or other problems in given stream.
                input.peek ();
            }

            return AssetState::READY;
        }

        std::filesystem::path yamlUniformsPath = EMERGENCE_BUILD_STRING (root, "/", _assetId, ".uniform.values.yaml");
        if (std::filesystem::exists (yamlUniformsPath))
        {
            std::ifstream input {yamlUniformsPath};
            bool successful = uniformValueBundleDeserializer.Begin (input);

            while (successful && uniformValueBundleDeserializer.HasNext ())
            {
                if ((successful = uniformValueBundleDeserializer.Next (&uniformItem)))
                {
                    insertDeserializedItem ();
                }
            }

            uniformValueBundleDeserializer.End ();
            if (!successful)
            {
                EMERGENCE_LOG (ERROR, "Material2dInstanceManagement: Unable to deserialize uniform values \"",
                               yamlUniformsPath.generic_string<char, std::char_traits<char>, Memory::HeapSTD<char>> (),
                               "\".");
                return AssetState::CORRUPTED;
            }

            return AssetState::READY;
        }
    }

    return AssetState::MISSING;
}

void Manager::CopyParentValuesIntoCollector (Memory::UniqueString _assetId) noexcept
{
    auto findByNameLambda = [] (const Memory::UniqueString _name)
    {
        return [_name] (const UniformValueBundleItem &_item)
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
            uniformValuesCollector.emplace_back (UniformValueBundleItem {uniform->uniformName, uniform->value});
        }
    }

    for (auto uniformCursor = fetchUniformMatrix3x3fValueById.Execute (&_assetId);
         const auto *uniform = static_cast<const UniformMatrix3x3fValue *> (*uniformCursor); ++uniformCursor)
    {
        if (std::find_if (uniformValuesCollector.begin (), uniformValuesCollector.end (),
                          findByNameLambda (uniform->uniformName)) == uniformValuesCollector.end ())
        {
            uniformValuesCollector.emplace_back (UniformValueBundleItem {uniform->uniformName, uniform->value});
        }
    }

    for (auto uniformCursor = fetchUniformMatrix4x4fValueById.Execute (&_assetId);
         const auto *uniform = static_cast<const UniformMatrix4x4fValue *> (*uniformCursor); ++uniformCursor)
    {
        if (std::find_if (uniformValuesCollector.begin (), uniformValuesCollector.end (),
                          findByNameLambda (uniform->uniformName)) == uniformValuesCollector.end ())
        {
            uniformValuesCollector.emplace_back (UniformValueBundleItem {uniform->uniformName, uniform->value});
        }
    }

    for (auto uniformCursor = fetchUniformSamplerValueById.Execute (&_assetId);
         const auto *uniform = static_cast<const UniformSamplerValue *> (*uniformCursor); ++uniformCursor)
    {
        if (std::find_if (uniformValuesCollector.begin (), uniformValuesCollector.end (),
                          findByNameLambda (uniform->uniformName)) == uniformValuesCollector.end ())
        {
            uniformValuesCollector.emplace_back (UniformValueBundleItem {uniform->uniformName, uniform->textureId});
        }
    }
}

void Manager::ProcessUnloading () noexcept
{
    for (auto eventCursor = fetchAssetRemovedEvents.Execute ();
         const auto *event = static_cast<const AssetRemovedNormalEvent *> (*eventCursor); ++eventCursor)
    {
        Unload (event->id);
    }
}

void Manager::Unload (Memory::UniqueString _assetId) noexcept
{
    if (auto materialInstanceLoadingStateCursor = removeMaterialInstanceLoadingStateById.Execute (&_assetId);
        materialInstanceLoadingStateCursor.ReadConst ())
    {
        ~materialInstanceLoadingStateCursor;
    }

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
                        uint64_t _maxLoadingTimePerFrameNs,
                        const AssetReferenceBindingEventMap &_eventMap) noexcept
{
    auto iterator = _eventMap.stateUpdate.find (Material2dInstance::Reflect ().mapping);
    if (iterator == _eventMap.stateUpdate.end ())
    {
        EMERGENCE_LOG (WARNING,
                       "Material2dInstanceManagement: Task not registered, because Material2dInstance  "
                       "is not found in state update map. Perhaps it is not referenced by anything?");
        return;
    }

    _pipelineBuilder.AddTask (Memory::UniqueString {"Material2dInstanceManager"})
        .SetExecutor<Manager> (_materialInstanceRootPaths, _maxLoadingTimePerFrameNs, iterator->second);
}
} // namespace Emergence::Celerity::Material2dInstanceManagement

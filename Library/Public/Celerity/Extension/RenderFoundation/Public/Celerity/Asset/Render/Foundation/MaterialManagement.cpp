#include <filesystem>
#include <fstream>

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

#include <Serialization/Binary.hpp>
#include <Serialization/Yaml.hpp>

namespace Emergence::Celerity::MaterialManagement
{
class Manager final : public TaskExecutorBase<Manager>, public StatefulAssetManagerBase<Manager>
{
public:
    using AssetType = Material;

    using LoadingState = MaterialLoadingState;

    Manager (TaskConstructor &_constructor,
             const Container::Vector<Memory::UniqueString> &_materialRootPaths,
             const Container::Vector<Memory::UniqueString> &_shaderRootPaths,
             const StandardLayout::Mapping &_stateUpdateEvent) noexcept;

private:
    friend class StatefulAssetManagerBase<Manager>;

    AssetState StartLoading (MaterialLoadingState *_loadingState) noexcept;

    AssetState TryFinishLoading (MaterialLoadingState *_loadingState) noexcept;

    Container::Vector<uint8_t> LoadShaderFile (const Container::String &_file) noexcept;

    bool RegisterUniform (Memory::UniqueString _assetId, const UniformDescription &_description) noexcept;

    void Unload (Memory::UniqueString _assetId) noexcept;

    InsertLongTermQuery insertMaterial;
    InsertLongTermQuery insertUniform;

    RemoveValueQuery removeMaterialById;
    RemoveValueQuery removeUniformById;

    Container::Vector<Memory::UniqueString> materialRootPaths {Memory::Profiler::AllocationGroup::Top ()};
    Container::Vector<Memory::UniqueString> shaderRootPaths {Memory::Profiler::AllocationGroup::Top ()};
};

Manager::Manager (TaskConstructor &_constructor,
                  const Container::Vector<Memory::UniqueString> &_materialRootPaths,
                  const Container::Vector<Memory::UniqueString> &_shaderRootPaths,
                  const StandardLayout::Mapping &_stateUpdateEvent) noexcept
    : StatefulAssetManagerBase (_constructor, _stateUpdateEvent),

      insertMaterial (INSERT_LONG_TERM (Material)),
      insertUniform (INSERT_LONG_TERM (Uniform)),

      removeMaterialById (REMOVE_VALUE_1F (Material, assetId)),
      removeUniformById (REMOVE_VALUE_1F (Uniform, assetId))
{
    materialRootPaths.reserve (_materialRootPaths.size ());
    for (Memory::UniqueString materialRoot : _materialRootPaths)
    {
        materialRootPaths.emplace_back (materialRoot);
    }

    shaderRootPaths.reserve (_shaderRootPaths.size ());
    for (Memory::UniqueString shaderRoot : _shaderRootPaths)
    {
        shaderRootPaths.emplace_back (shaderRoot);
    }

    _constructor.DependOn (AssetManagement::Checkpoint::ASSET_LOADING_STARTED);
    _constructor.MakeDependencyOf (AssetManagement::Checkpoint::ASSET_LOADING_FINISHED);
}

AssetState Manager::StartLoading (MaterialLoadingState *_loadingState) noexcept
{
    for (Memory::UniqueString root : materialRootPaths)
    {
        std::filesystem::path binaryMaterialPath =
            EMERGENCE_BUILD_STRING (root, "/", _loadingState->assetId, ".material.bin");
        if (std::filesystem::exists (binaryMaterialPath))
        {
            Job::Dispatcher::Global ().Dispatch (
                Job::Priority::BACKGROUND,
                [sharedState {_loadingState->sharedState}, binaryMaterialPath] ()
                {
                    std::ifstream input {binaryMaterialPath, std::ios::binary};
                    if (!Serialization::Binary::DeserializeObject (input, &sharedState->asset,
                                                                   MaterialAsset::Reflect ().mapping, {}))
                    {
                        EMERGENCE_LOG (
                            ERROR, "MaterialManagement: Unable to load material from \"",
                            binaryMaterialPath.generic_string<char, std::char_traits<char>, Memory::HeapSTD<char>> (),
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

        std::filesystem::path yamlMaterialPath =
            EMERGENCE_BUILD_STRING (root, "/", _loadingState->assetId, ".material.yaml");
        if (std::filesystem::exists (yamlMaterialPath))
        {
            Job::Dispatcher::Global ().Dispatch (
                Job::Priority::BACKGROUND,
                [sharedState {_loadingState->sharedState}, yamlMaterialPath] ()
                {
                    std::ifstream input {yamlMaterialPath};
                    if (!Serialization::Yaml::DeserializeObject (input, &sharedState->asset,
                                                                 MaterialAsset::Reflect ().mapping, {}))
                    {
                        EMERGENCE_LOG (
                            ERROR, "MaterialManagement: Unable to load material from \"",
                            yamlMaterialPath.generic_string<char, std::char_traits<char>, Memory::HeapSTD<char>> (),
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

    EMERGENCE_LOG (ERROR, "MaterialManagement: Unable to find material \"", _loadingState->assetId, "\".");
    return AssetState::MISSING;
}

AssetState Manager::TryFinishLoading (MaterialLoadingState *_loadingState) noexcept
{
    auto materialCursor = insertMaterial.Execute ();
    auto *material = static_cast<Material *> (++materialCursor);
    material->assetId = _loadingState->assetId;
    material->vertexShader = _loadingState->sharedState->asset.vertexShader;
    material->fragmentShader = _loadingState->sharedState->asset.fragmentShader;

    const Container::Vector<uint8_t> vertexShader =
        LoadShaderFile (EMERGENCE_BUILD_STRING (material->vertexShader, ".vertex"));
    const Container::Vector<uint8_t> fragmentShader =
        LoadShaderFile (EMERGENCE_BUILD_STRING (material->fragmentShader, ".fragment"));
    material->program = {vertexShader.data (), vertexShader.size (), fragmentShader.data (), fragmentShader.size ()};

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

Container::Vector<uint8_t> Manager::LoadShaderFile (const Container::String &_file) noexcept
{
    Container::Vector<uint8_t> result {Render::Backend::GetSharedAllocationGroup ()};
    for (Memory::UniqueString shaderRoot : shaderRootPaths)
    {
        std::filesystem::path shaderPath =
            EMERGENCE_BUILD_STRING (shaderRoot, "/", _file, Render::Backend::Program::GetShaderSuffix ());

        if (std::filesystem::exists (shaderPath))
        {
            std::ifstream input {shaderPath, std::ios::binary | std::ios::ate};
            std::streamsize fileSize = input.tellg ();
            input.seekg (0u, std::ios::beg);

            result.resize (fileSize);
            input.read (reinterpret_cast<char *> (result.data ()), fileSize);
            break;
        }
    }

    return result;
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
                        const Container::Vector<Memory::UniqueString> &_materialRootPaths,
                        const Container::Vector<Memory::UniqueString> &_shaderRootPaths,
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
        .SetExecutor<Manager> (_materialRootPaths, _shaderRootPaths, iterator->second);
}
} // namespace Emergence::Celerity::MaterialManagement

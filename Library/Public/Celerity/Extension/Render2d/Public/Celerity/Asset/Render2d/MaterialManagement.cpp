#include <filesystem>
#include <fstream>

#include <Celerity/Asset/AssetManagement.hpp>
#include <Celerity/Asset/Events.hpp>
#include <Celerity/Asset/Render2d/Material.hpp>
#include <Celerity/Asset/Render2d/MaterialManagement.hpp>
#include <Celerity/PipelineBuilderMacros.hpp>

#include <Log/Log.hpp>

#include <Render/Backend/Configuration.hpp>

#include <Serialization/Binary.hpp>
#include <Serialization/Yaml.hpp>

#include <SyntaxSugar/Time.hpp>

namespace Emergence::Celerity::MaterialManagement
{
class Manager final : public TaskExecutorBase<Manager>
{
public:
    Manager (TaskConstructor &_constructor,
             const Container::Vector<Memory::UniqueString> &_materialRootPaths,
             const Container::Vector<Memory::UniqueString> &_shaderRootPaths,
             uint64_t _maxLoadingTimePerFrameNs,
             const StandardLayout::Mapping &_stateUpdateEvent) noexcept;

    void Execute () noexcept;

private:
    void ProcessLoading () noexcept;

    AssetState LoadMaterial (Memory::UniqueString _assetId) noexcept;

    AssetState LoadUniforms (Memory::UniqueString _assetId) noexcept;

    Container::Vector<uint8_t> LoadShaderFile (const Container::String &_file) noexcept;

    bool RegisterUniform (Memory::UniqueString _assetId, const UniformBundleItem &_bundleItem) noexcept;

    void ProcessUnloading () noexcept;

    void Unload (Memory::UniqueString _assetId) noexcept;

    InsertShortTermQuery insertAssetStateEvent;
    FetchSequenceQuery fetchAssetRemovedEvents;
    FetchValueQuery fetchAssetByTypeNumberAndState;

    InsertLongTermQuery insertMaterial;
    InsertLongTermQuery insertUniform;
    RemoveValueQuery removeMaterialById;
    RemoveValueQuery removeUniformById;

    Container::Vector<Memory::UniqueString> materialRootPaths {Memory::Profiler::AllocationGroup::Top ()};
    Container::Vector<Memory::UniqueString> shaderRootPaths {Memory::Profiler::AllocationGroup::Top ()};
    const uint64_t maxLoadingTimePerFrameNs;

    Serialization::FieldNameLookupCache materialAssetLookupCache {MaterialAssetHeader::Reflect ().mapping};
    Serialization::Yaml::ObjectBundleDeserializer uniformBundleDeserializer {UniformBundleItem::Reflect ().mapping};
};

Manager::Manager (TaskConstructor &_constructor,
                  const Container::Vector<Memory::UniqueString> &_materialRootPaths,
                  const Container::Vector<Memory::UniqueString> &_shaderRootPaths,
                  uint64_t _maxLoadingTimePerFrameNs,
                  const StandardLayout::Mapping &_stateUpdateEvent) noexcept
    : insertAssetStateEvent (_constructor.InsertShortTerm (_stateUpdateEvent)),
      fetchAssetRemovedEvents (FETCH_SEQUENCE (AssetRemovedNormalEvent)),
      fetchAssetByTypeNumberAndState (FETCH_VALUE_2F (Asset, typeNumber, state)),

      insertMaterial (INSERT_LONG_TERM (Material)),
      insertUniform (INSERT_LONG_TERM (Uniform)),
      removeMaterialById (REMOVE_VALUE_1F (Material, assetId)),
      removeUniformById (REMOVE_VALUE_1F (Uniform, assetId)),

      maxLoadingTimePerFrameNs (_maxLoadingTimePerFrameNs)
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

void Manager::Execute () noexcept
{
    ProcessLoading ();
    ProcessUnloading ();
}

void Manager::ProcessLoading () noexcept
{
    struct
    {
        StandardLayout::Mapping mapping = Material::Reflect ().mapping;
        AssetState state = AssetState::LOADING;
    } loadingMaterialsParameter;

    const std::uint64_t startTime = Emergence::Time::NanosecondsSinceStartup ();
    for (auto assetCursor = fetchAssetByTypeNumberAndState.Execute (&loadingMaterialsParameter);
         const auto *asset = static_cast<const Asset *> (*assetCursor); ++assetCursor)
    {
        if (Emergence::Time::NanosecondsSinceStartup () - startTime > maxLoadingTimePerFrameNs)
        {
            break;
        }

        // If we're reloading material, unload old one first.
        Unload (asset->id);

        auto eventCursor = insertAssetStateEvent.Execute ();
        auto *event = static_cast<AssetStateUpdateEventView *> (++eventCursor);
        event->assetId = asset->id;
        event->state = LoadMaterial (asset->id);

        if (event->state == AssetState::READY)
        {
            event->state = LoadUniforms (asset->id);
        }

        EMERGENCE_ASSERT (event->state != AssetState::LOADING);
    }
}

AssetState Manager::LoadMaterial (Memory::UniqueString _assetId) noexcept
{
    MaterialAssetHeader materialAsset;
    bool loaded = false;

    for (Memory::UniqueString root : materialRootPaths)
    {
        std::filesystem::path binaryMaterialPath = EMERGENCE_BUILD_STRING (root, "/", _assetId, ".material.bin");
        if (std::filesystem::exists (binaryMaterialPath))
        {
            std::ifstream input {binaryMaterialPath, std::ios::binary};
            if (!Serialization::Binary::DeserializeObject (input, &materialAsset,
                                                           MaterialAssetHeader::Reflect ().mapping))
            {
                EMERGENCE_LOG (
                    ERROR, "MaterialManagement: Unable to load material from \"",
                    binaryMaterialPath.generic_string<char, std::char_traits<char>, Memory::HeapSTD<char>> (), "\".");
                return AssetState::CORRUPTED;
            }

            loaded = true;
            break;
        }

        std::filesystem::path yamlMaterialPath = EMERGENCE_BUILD_STRING (root, "/", _assetId, ".material.yaml");
        if (std::filesystem::exists (yamlMaterialPath))
        {
            std::ifstream input {yamlMaterialPath};
            if (!Serialization::Yaml::DeserializeObject (input, &materialAsset, materialAssetLookupCache))
            {
                EMERGENCE_LOG (ERROR, "MaterialManagement: Unable to load material from \"",
                               yamlMaterialPath.generic_string<char, std::char_traits<char>, Memory::HeapSTD<char>> (),
                               "\".");
                return AssetState::CORRUPTED;
            }

            loaded = true;
            break;
        }
    }

    if (!loaded)
    {
        EMERGENCE_LOG (ERROR, "MaterialManagement: Unable to find material \"", _assetId, "\".");
        return AssetState::MISSING;
    }

    auto materialCursor = insertMaterial.Execute ();
    auto *material = static_cast<Material *> (++materialCursor);
    material->assetId = _assetId;
    material->vertexShader = materialAsset.vertexShader;
    material->fragmentShader = materialAsset.fragmentShader;

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

    return AssetState::READY;
}

AssetState Manager::LoadUniforms (Memory::UniqueString _assetId) noexcept
{
    UniformBundleItem uniformItem;
    for (Memory::UniqueString root : materialRootPaths)
    {
        std::filesystem::path binaryUniformsPath = EMERGENCE_BUILD_STRING (root, "/", _assetId, ".uniforms.bin");
        if (std::filesystem::exists (binaryUniformsPath))
        {
            std::ifstream input {binaryUniformsPath, std::ios::binary};
            // We need to do get-unget in order to force empty file check. Otherwise, it is not guaranteed.
            input.get ();
            input.unget ();

            while (input)
            {
                if (!Serialization::Binary::DeserializeObject (input, &uniformItem,
                                                               UniformBundleItem::Reflect ().mapping))
                {
                    EMERGENCE_LOG (
                        ERROR, "MaterialManagement: Unable to deserialize uniform bundle \"",
                        binaryUniformsPath.generic_string<char, std::char_traits<char>, Memory::HeapSTD<char>> (),
                        "\".");
                    return AssetState::CORRUPTED;
                }

                RegisterUniform (_assetId, uniformItem);
                // Use peek to test for the end of file or other problems in given stream.
                input.peek ();
            }

            return AssetState::READY;
        }

        std::filesystem::path yamlUniformsPath = EMERGENCE_BUILD_STRING (root, "/", _assetId, ".uniforms.yaml");
        if (std::filesystem::exists (yamlUniformsPath))
        {
            std::ifstream input {yamlUniformsPath};
            bool successful = uniformBundleDeserializer.Begin (input);

            while (successful && uniformBundleDeserializer.HasNext ())
            {
                if ((successful = uniformBundleDeserializer.Next (&uniformItem)))
                {
                    RegisterUniform (_assetId, uniformItem);
                }
            }

            uniformBundleDeserializer.End ();
            if (!successful)
            {
                EMERGENCE_LOG (ERROR, "MaterialManagement: Unable to deserialize uniform bundle \"",
                               yamlUniformsPath.generic_string<char, std::char_traits<char>, Memory::HeapSTD<char>> (),
                               "\".");
                return AssetState::CORRUPTED;
            }

            return AssetState::READY;
        }
    }

    return AssetState::MISSING;
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

bool Manager::RegisterUniform (Memory::UniqueString _assetId, const UniformBundleItem &_bundleItem) noexcept
{
    Render::Backend::Uniform nativeUniform {_bundleItem.name, _bundleItem.type};
    if (!nativeUniform.IsValid ())
    {
        EMERGENCE_LOG (ERROR, "MaterialManagement: Unable to register uniform \"", _bundleItem.name, "\".");
        return false;
    }

    auto cursor = insertUniform.Execute ();
    auto *uniform = static_cast<Uniform *> (++cursor);

    uniform->assetId = _assetId;
    uniform->name = _bundleItem.name;
    uniform->type = _bundleItem.type;
    uniform->uniform = std::move (nativeUniform);

    if (_bundleItem.type == Render::Backend::UniformType::SAMPLER)
    {
        uniform->textureStage = _bundleItem.textureStage;
    }

    return true;
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
                        uint64_t _maxLoadingTimePerFrameNs,
                        const AssetReferenceBindingEventMap &_eventMap) noexcept
{
    auto iterator = _eventMap.stateUpdate.find (Material::Reflect ().mapping);
    if (iterator == _eventMap.stateUpdate.end ())
    {
        EMERGENCE_LOG (WARNING,
                       "MaterialManagement: Task not registered, because Material2d is not found "
                       "in state update map. Perhaps it is not referenced by anything?");
        return;
    }

    auto visualGroup = _pipelineBuilder.OpenVisualGroup ("MaterialManagement");
    _pipelineBuilder.AddTask (Memory::UniqueString {"MaterialManager"})
        .SetExecutor<Manager> (_materialRootPaths, _shaderRootPaths, _maxLoadingTimePerFrameNs, iterator->second);
}
} // namespace Emergence::Celerity::MaterialManagement

#include <filesystem>
#include <fstream>

#include <bgfx/bgfx.h>

#include <Celerity/Asset/Events.hpp>
#include <Celerity/Asset/Render2d/Material2d.hpp>
#include <Celerity/Asset/Render2d/Material2dManagement.hpp>
#include <Celerity/PipelineBuilderMacros.hpp>

#include <Log/Log.hpp>

#include <Serialization/Binary.hpp>
#include <Serialization/Yaml.hpp>

#include <SyntaxSugar/Time.hpp>

namespace Emergence::Celerity::Material2dManagement
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

    bgfx::ShaderHandle LoadShader (Memory::UniqueString _id, bool _isFragment) noexcept;

    bool RegisterUniform (Memory::UniqueString _assetId, const Uniform2dBundleItem &_bundleItem) noexcept;

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

    Serialization::FieldNameLookupCache materialAssetLookupCache {Material2dAssetHeader::Reflect ().mapping};
    Serialization::Yaml::ObjectBundleDeserializer uniformBundleDeserializer {Uniform2dBundleItem::Reflect ().mapping};
};

Manager::Manager (TaskConstructor &_constructor,
                  const Container::Vector<Memory::UniqueString> &_materialRootPaths,
                  const Container::Vector<Memory::UniqueString> &_shaderRootPaths,
                  uint64_t _maxLoadingTimePerFrameNs,
                  const StandardLayout::Mapping &_stateUpdateEvent) noexcept
    : insertAssetStateEvent (_constructor.InsertShortTerm (_stateUpdateEvent)),
      fetchAssetRemovedEvents (FETCH_SEQUENCE (AssetRemovedNormalEvent)),
      fetchAssetByTypeNumberAndState (FETCH_VALUE_2F (Asset, typeNumber, state)),

      insertMaterial (INSERT_LONG_TERM (Material2d)),
      insertUniform (INSERT_LONG_TERM (Uniform2d)),
      removeMaterialById (REMOVE_VALUE_1F (Material2d, assetId)),
      removeUniformById (REMOVE_VALUE_1F (Uniform2d, assetId)),

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
        StandardLayout::Mapping mapping = Material2d::Reflect ().mapping;
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
    }
}

AssetState Manager::LoadMaterial (Memory::UniqueString _assetId) noexcept
{
    Material2dAssetHeader materialAsset;
    bool loaded = false;

    for (Memory::UniqueString root : materialRootPaths)
    {
        std::filesystem::path binaryMaterialPath = EMERGENCE_BUILD_STRING (root, "/", _assetId, ".material.bin");
        if (std::filesystem::exists (binaryMaterialPath))
        {
            std::ifstream input {binaryMaterialPath, std::ios::binary};
            if (!Serialization::Binary::DeserializeObject (input, &materialAsset,
                                                           Material2dAssetHeader::Reflect ().mapping))
            {
                EMERGENCE_LOG (
                    ERROR, "Material2dManagement: Unable to load material from \"",
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
                EMERGENCE_LOG (ERROR, "Material2dManagement: Unable to load material from \"",
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
        return AssetState::MISSING;
    }

    auto materialCursor = insertMaterial.Execute ();
    auto *material = static_cast<Material2d *> (++materialCursor);
    material->assetId = _assetId;
    material->vertexShader = materialAsset.vertexShader;
    material->fragmentShader = materialAsset.fragmentShader;

    bgfx::ShaderHandle vertexShader = LoadShader (material->vertexShader, false);
    if (!bgfx::isValid (vertexShader))
    {
        EMERGENCE_LOG (ERROR, "Material2dManagement: Unable to load vertex shader \"", material->vertexShader, "\".");
        return AssetState::CORRUPTED;
    }

    bgfx::ShaderHandle fragmentShader = LoadShader (material->fragmentShader, true);
    if (!bgfx::isValid (fragmentShader))
    {
        EMERGENCE_LOG (ERROR, "Material2dManagement: Unable to load fragment shader \"", material->fragmentShader,
                       "\".");
        return AssetState::CORRUPTED;
    }

    bgfx::ProgramHandle program = bgfx::createProgram (vertexShader, fragmentShader, true);
    if (!bgfx::isValid (program))
    {
        EMERGENCE_LOG (ERROR, "Material2dManagement: Unable to create program for material \"", material->assetId,
                       "\".");
        return AssetState::CORRUPTED;
    }

    material->nativeHandle = program.idx;
    return AssetState::READY;
}

AssetState Manager::LoadUniforms (Memory::UniqueString _assetId) noexcept
{
    Uniform2dBundleItem uniformItem;
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
                                                               Uniform2dBundleItem::Reflect ().mapping))
                {
                    EMERGENCE_LOG (
                        ERROR, "Material2dManagement: Unable to deserialize uniform bundle \"",
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
                EMERGENCE_LOG (ERROR, "Material2dManagement: Unable to deserialize uniform bundle \"",
                               yamlUniformsPath.generic_string<char, std::char_traits<char>, Memory::HeapSTD<char>> (),
                               "\".");
                return AssetState::CORRUPTED;
            }

            return AssetState::READY;
        }
    }

    return AssetState::MISSING;
}

bgfx::ShaderHandle Manager::LoadShader (Memory::UniqueString _id, bool _isFragment) noexcept
{
    const char *typeSuffix = _isFragment ? ".fragment" : ".vertex";
    const char *platformSuffix = nullptr;

    switch (bgfx::getRendererType ())
    {
    case bgfx::RendererType::Noop:
        platformSuffix = ".noop";
        break;

    case bgfx::RendererType::Agc:
        platformSuffix = ".agc";
        break;

    case bgfx::RendererType::Direct3D9:
        platformSuffix = ".dx9";
        break;

    case bgfx::RendererType::Direct3D11:
    case bgfx::RendererType::Direct3D12:
        platformSuffix = ".dx11";
        break;

    case bgfx::RendererType::Gnm:
        platformSuffix = ".pssl";
        break;

    case bgfx::RendererType::Metal:
        platformSuffix = ".metal";
        break;

    case bgfx::RendererType::Nvn:
        platformSuffix = ".nvn";
        break;

    case bgfx::RendererType::OpenGLES:
        platformSuffix = ".essl";
        break;

    case bgfx::RendererType::OpenGL:
        platformSuffix = ".glsl";
        break;

    case bgfx::RendererType::Vulkan:
        platformSuffix = ".spirv";
        break;

    case bgfx::RendererType::WebGPU:
        platformSuffix = ".wssl";
        break;

    case bgfx::RendererType::Count:
        break;
    }

    if (!platformSuffix)
    {
        return bgfx::ShaderHandle {bgfx::kInvalidHandle};
    }

    for (Memory::UniqueString shaderRoot : shaderRootPaths)
    {
        std::filesystem::path shaderPath = EMERGENCE_BUILD_STRING (shaderRoot, "/", _id, typeSuffix, platformSuffix);
        if (std::filesystem::exists (shaderPath))
        {
            std::ifstream input {shaderPath, std::ios::binary | std::ios::ate};
            std::streamsize fileSize = input.tellg ();
            input.seekg (0u, std::ios::beg);

            const bgfx::Memory *shaderMemory = bgfx::alloc (fileSize + 1);
            input.read (reinterpret_cast<char *> (shaderMemory->data), fileSize);
            shaderMemory->data[shaderMemory->size - 1] = '\0';
            return bgfx::createShader (shaderMemory);
        }
    }

    return bgfx::ShaderHandle {bgfx::kInvalidHandle};
}

bool Manager::RegisterUniform (Memory::UniqueString _assetId, const Uniform2dBundleItem &_bundleItem) noexcept
{
    bgfx::UniformType::Enum uniformType;
    switch (_bundleItem.type)
    {
    case Uniform2dType::VECTOR_4F:
        uniformType = bgfx::UniformType::Vec4;
        break;

    case Uniform2dType::MATRIX_3X3F:
        uniformType = bgfx::UniformType::Mat3;
        break;

    case Uniform2dType::MATRIX_4X4F:
        uniformType = bgfx::UniformType::Mat4;
        break;

    case Uniform2dType::SAMPLER:
        uniformType = bgfx::UniformType::Sampler;
        break;
    }

    bgfx::UniformHandle uniformHandle = bgfx::createUniform (*_bundleItem.name, uniformType);
    if (!bgfx::isValid (uniformHandle))
    {
        EMERGENCE_LOG (ERROR, "Material2dManagement: Unable to register uniform \"", _bundleItem.name, "\".");
        return false;
    }

    auto cursor = insertUniform.Execute ();
    auto *uniform = static_cast<Uniform2d *> (++cursor);

    uniform->assetId = _assetId;
    uniform->name = _bundleItem.name;
    uniform->type = _bundleItem.type;
    uniform->nativeHandle = uniformHandle.idx;
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
    for (auto uniformCursor = removeUniformById.Execute (&_assetId);
         const auto *uniform = static_cast<const Uniform2d *> (uniformCursor.ReadConst ()); ~uniformCursor)
    {
        bgfx::UniformHandle handle {static_cast<uint16_t> (uniform->nativeHandle)};
        bgfx::destroy (handle);
    }

    if (auto materialCursor = removeMaterialById.Execute (&_assetId);
        const auto *material = static_cast<const Material2d *> (materialCursor.ReadConst ()))
    {
        bgfx::ProgramHandle handle {static_cast<uint16_t> (material->nativeHandle)};
        bgfx::destroy (handle);
        ~materialCursor;
    }
}

void AddToNormalUpdate (PipelineBuilder &_pipelineBuilder,
                        const Container::Vector<Memory::UniqueString> &_materialRootPaths,
                        const Container::Vector<Memory::UniqueString> &_shaderRootPaths,
                        uint64_t _maxLoadingTimePerFrameNs,
                        const AssetReferenceBindingEventMap &_eventMap) noexcept
{
    auto iterator = _eventMap.stateUpdate.find (Material2d::Reflect ().mapping);
    if (iterator == _eventMap.stateUpdate.end ())
    {
        EMERGENCE_LOG (WARNING,
                       "Material2dManagement: Task not registered, because Material2d is not found "
                       "in state update map. Perhaps it is not referenced by anything?");
        return;
    }

    _pipelineBuilder.AddTask (Memory::UniqueString {"Material2dManagement"})
        .SetExecutor<Manager> (_materialRootPaths, _shaderRootPaths, _maxLoadingTimePerFrameNs, iterator->second);
}
} // namespace Emergence::Celerity::Material2dManagement

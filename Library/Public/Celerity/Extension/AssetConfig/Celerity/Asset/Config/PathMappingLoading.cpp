#include <filesystem>
#include <fstream>

#include <Celerity/Asset/Config/LoadingStateSingleton.hpp>
#include <Celerity/Asset/Config/PathMappingLoading.hpp>
#include <Celerity/PipelineBuilderMacros.hpp>

#include <Log/Log.hpp>

#include <Serialization/Binary.hpp>
#include <Serialization/Yaml.hpp>

namespace Emergence::Celerity::AssetConfigPathMappingLoading
{
const Memory::UniqueString Checkpoint::STARTED {"AssetConfigPathMappingLoadingStarted"};
const Memory::UniqueString Checkpoint::FINISHED {"AssetConfigPathMappingLoadingFinished"};

const char *const BINARY_FILE_NAME = "ConfigPathMapping.bin";
const char *const YAML_FILE_NAME = "ConfigPathMapping.yaml";

class Loader final : public TaskExecutorBase<Loader>
{
public:
    Loader (TaskConstructor &_constructor,
            Container::String _pathToAssetRoot,
            const Container::Vector<AssetConfigTypeMeta> &_supportedTypes) noexcept;

    void Execute () noexcept;

private:
    void RegisterPathMapping (AssetConfigLoadingStateSingleton *_state, const ListItem &_item) const noexcept;

    ModifySingletonQuery modifyState;

    Container::String pathToAssetRoot;
    Container::Vector<AssetConfigTypeMeta> supportedTypes {Memory::Profiler::AllocationGroup::Top ()};
};

Loader::Loader (TaskConstructor &_constructor,
                Container::String _pathToAssetRoot,
                const Container::Vector<AssetConfigTypeMeta> &_supportedTypes) noexcept
    : modifyState (MODIFY_SINGLETON (AssetConfigLoadingStateSingleton)),
      pathToAssetRoot (std::move (_pathToAssetRoot))
{
    _constructor.DependOn (Checkpoint::STARTED);
    _constructor.MakeDependencyOf (Checkpoint::FINISHED);
    supportedTypes.reserve (_supportedTypes.size ());

    for (const AssetConfigTypeMeta &type : _supportedTypes)
    {
        supportedTypes.emplace_back (type);
    }
}

void Loader::Execute () noexcept
{
    auto stateCursor = modifyState.Execute ();
    auto *state = static_cast<AssetConfigLoadingStateSingleton *> (*stateCursor);

    if (state->pathMappingLoaded)
    {
        return;
    }

    // Path mapping are usually quite small, because there is not a lot of config types
    // (not more than 100), therefore we can safely load everything during single frame.

    std::filesystem::path binaryPath {EMERGENCE_BUILD_STRING (pathToAssetRoot, "/", BINARY_FILE_NAME)};
    std::filesystem::path yamlPath {EMERGENCE_BUILD_STRING (pathToAssetRoot, "/", YAML_FILE_NAME)};
    ListItem item;

    if (std::filesystem::exists (binaryPath))
    {
        std::ifstream input {binaryPath, std::ios::binary};
        while (input)
        {
            if (!Serialization::Binary::DeserializeObject (input, &item, ListItem::Reflect ().mapping))
            {
                EMERGENCE_LOG (ERROR, "AssetConfigPathMappingLoading: Unable to deserialize config path mapping \"",
                               binaryPath.generic_string<char, std::char_traits<char>, Memory::HeapSTD<char>> (),
                               "\".");
                break;
            }

            RegisterPathMapping (state, item);
            // Use peek to test for the end of file or other problems in given stream.
            input.peek ();
        }
    }
    else if (std::filesystem::exists (yamlPath))
    {
        std::ifstream input {yamlPath};
        Serialization::Yaml::ObjectBundleDeserializer deserializer {ListItem::Reflect ().mapping};
        bool successful = deserializer.Begin (input);

        while (successful && deserializer.HasNext ())
        {
            if ((successful = deserializer.Next (&item)))
            {
                RegisterPathMapping (state, item);
            }
        }

        deserializer.End ();
        if (!successful)
        {
            EMERGENCE_LOG (ERROR, "AssetConfigPathMappingLoading: Unable to deserialize config path mapping \"",
                           yamlPath.generic_string<char, std::char_traits<char>, Memory::HeapSTD<char>> (), "\".");
        }
    }
    else
    {
        EMERGENCE_LOG (ERROR, "AssetConfigPathMappingLoading: Unable to find path mapping file!");
    }

    // Release all temporary occupied memory: after loading we don't need it.
    pathToAssetRoot.clear ();
    pathToAssetRoot.shrink_to_fit ();

    supportedTypes.clear ();
    supportedTypes.shrink_to_fit ();

    state->pathMappingLoaded = true;
}

void Loader::RegisterPathMapping (AssetConfigLoadingStateSingleton *_state, const ListItem &_item) const noexcept
{
    for (const AssetConfigTypeMeta &type : supportedTypes)
    {
        if (type.mapping.GetName () == _item.typeName)
        {
            AssetConfigLoadingStateSingleton::TypeState &typeState = _state->typeStates.emplace_back ();
            typeState.type = type.mapping;
            typeState.nameField = type.nameField;
            typeState.folder = EMERGENCE_BUILD_STRING (pathToAssetRoot, "/", _item.folder.data ());
            return;
        }
    }

    EMERGENCE_LOG (ERROR, "AssetConfigPathMappingLoading: Config type \"", _item.typeName, "\" is not supported!");
}

void AddToLoadingPipeline (PipelineBuilder &_builder,
                           Container::String _assetRootPath,
                           const Container::Vector<AssetConfigTypeMeta> &_supportedTypes) noexcept
{
    auto visualGroup = _builder.OpenVisualGroup ("AssetConfigPathMappingLoading");
    _builder.AddCheckpoint (Checkpoint::STARTED);
    _builder.AddCheckpoint (Checkpoint::FINISHED);
    _builder.AddTask (Memory::UniqueString {"AssetConfigPathMappingLoader"})
        .SetExecutor<Loader> (std::move (_assetRootPath), _supportedTypes);
}

const ListItem::Reflection &ListItem::Reflect () noexcept
{
    static Reflection reflection = [] ()
    {
        EMERGENCE_MAPPING_REGISTRATION_BEGIN (ListItem);
        EMERGENCE_MAPPING_REGISTER_REGULAR (typeName);
        EMERGENCE_MAPPING_REGISTER_STRING (folder);
        EMERGENCE_MAPPING_REGISTRATION_END ();
    }();

    return reflection;
}
} // namespace Emergence::Celerity::AssetConfigPathMappingLoading

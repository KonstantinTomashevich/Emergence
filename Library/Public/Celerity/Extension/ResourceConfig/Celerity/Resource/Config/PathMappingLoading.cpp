#include <filesystem>
#include <fstream>

#include <Celerity/PipelineBuilderMacros.hpp>
#include <Celerity/Resource/Config/LoadingStateSingleton.hpp>
#include <Celerity/Resource/Config/PathMappingLoading.hpp>

#include <Log/Log.hpp>

#include <Serialization/Binary.hpp>
#include <Serialization/Yaml.hpp>

namespace Emergence::Celerity::ResourceConfigPathMappingLoading
{
const Memory::UniqueString Checkpoint::STARTED {"ResourceConfigPathMappingLoadingStarted"};
const Memory::UniqueString Checkpoint::FINISHED {"ResourceConfigPathMappingLoadingFinished"};

const char *const BINARY_FILE_NAME = "ConfigPathMapping.bin";
const char *const YAML_FILE_NAME = "ConfigPathMapping.yaml";

class Loader final : public TaskExecutorBase<Loader>
{
public:
    Loader (TaskConstructor &_constructor,
            Container::String _pathToAssetRoot,
            const Container::Vector<ResourceConfigTypeMeta> &_supportedTypes) noexcept;

    void Execute () noexcept;

private:
    void RegisterPathMapping (ResourceConfigLoadingStateSingleton *_state,
                              const PathMapping &_pathMapping) const noexcept;

    ModifySingletonQuery modifyState;

    Container::String pathToAssetRoot;
    Container::Vector<ResourceConfigTypeMeta> supportedTypes {Memory::Profiler::AllocationGroup::Top ()};
};

Loader::Loader (TaskConstructor &_constructor,
                Container::String _pathToAssetRoot,
                const Container::Vector<ResourceConfigTypeMeta> &_supportedTypes) noexcept
    : modifyState (MODIFY_SINGLETON (ResourceConfigLoadingStateSingleton)),
      pathToAssetRoot (std::move (_pathToAssetRoot))
{
    _constructor.DependOn (Checkpoint::STARTED);
    _constructor.MakeDependencyOf (Checkpoint::FINISHED);
    supportedTypes.reserve (_supportedTypes.size ());

    for (const ResourceConfigTypeMeta &type : _supportedTypes)
    {
        supportedTypes.emplace_back (type);
    }
}

void Loader::Execute () noexcept
{
    auto stateCursor = modifyState.Execute ();
    auto *state = static_cast<ResourceConfigLoadingStateSingleton *> (*stateCursor);

    if (state->pathMappingLoaded)
    {
        return;
    }

    // Path mapping are usually quite small, because there is not a lot of config types
    // (not more than 100), therefore we can safely load everything during single frame.

    std::filesystem::path binaryPath {EMERGENCE_BUILD_STRING (pathToAssetRoot, "/", BINARY_FILE_NAME)};
    std::filesystem::path yamlPath {EMERGENCE_BUILD_STRING (pathToAssetRoot, "/", YAML_FILE_NAME)};
    PathMapping pathMapping;

    if (std::filesystem::exists (binaryPath))
    {
        std::ifstream input {binaryPath, std::ios::binary};
        if (!Serialization::Binary::DeserializeObject (input, &pathMapping, PathMapping::Reflect ().mapping, {}))
        {
            EMERGENCE_LOG (ERROR, "ResourceConfigPathMappingLoading: Unable to deserialize config path mapping \"",
                           binaryPath.generic_string<char, std::char_traits<char>, Memory::HeapSTD<char>> (), "\".");
        }

        RegisterPathMapping (state, pathMapping);
    }
    else if (std::filesystem::exists (yamlPath))
    {
        std::ifstream input {yamlPath};
        if (!Serialization::Yaml::DeserializeObject (input, &pathMapping, PathMapping::Reflect ().mapping, {}))
        {
            EMERGENCE_LOG (ERROR, "ResourceConfigPathMappingLoading: Unable to deserialize config path mapping \"",
                           yamlPath.generic_string<char, std::char_traits<char>, Memory::HeapSTD<char>> (), "\".");
        }

        RegisterPathMapping (state, pathMapping);
    }
    else
    {
        EMERGENCE_LOG (ERROR, "ResourceConfigPathMappingLoading: Unable to find path mapping file!");
    }

    // Release all temporary occupied memory: after loading we don't need it.
    pathToAssetRoot.clear ();
    pathToAssetRoot.shrink_to_fit ();

    supportedTypes.clear ();
    supportedTypes.shrink_to_fit ();

    state->pathMappingLoaded = true;
}

void Loader::RegisterPathMapping (ResourceConfigLoadingStateSingleton *_state,
                                  const PathMapping &_pathMapping) const noexcept
{
    for (const ListItem &item : _pathMapping.configs)
    {
        bool found = false;
        for (const ResourceConfigTypeMeta &type : supportedTypes)
        {
            if (type.mapping.GetName () == item.typeName)
            {
                ResourceConfigLoadingStateSingleton::TypeState &typeState = _state->typeStates.emplace_back ();
                typeState.type = type.mapping;
                typeState.nameField = type.nameField;
                typeState.folder = EMERGENCE_BUILD_STRING (pathToAssetRoot, "/", item.folder.data ());
                found = true;
                break;
            }
        }

        if (!found)
        {
            EMERGENCE_LOG (ERROR, "ResourceConfigPathMappingLoading: Config type \"", item.typeName,
                           "\" is not supported!");
        }
    }
}

void AddToLoadingPipeline (PipelineBuilder &_builder,
                           Container::String _resourceRootPath,
                           const Container::Vector<ResourceConfigTypeMeta> &_supportedTypes) noexcept
{
    auto visualGroup = _builder.OpenVisualGroup ("ResourceConfigPathMappingLoading");
    _builder.AddCheckpoint (Checkpoint::STARTED);
    _builder.AddCheckpoint (Checkpoint::FINISHED);
    _builder.AddTask (Memory::UniqueString {"ResourceConfigPathMappingLoader"})
        .SetExecutor<Loader> (std::move (_resourceRootPath), _supportedTypes);
}

const ListItem::Reflection &ListItem::Reflect () noexcept
{
    static Reflection reflection = [] ()
    {
        EMERGENCE_MAPPING_REGISTRATION_BEGIN (ListItem);
        EMERGENCE_MAPPING_REGISTER_REGULAR (typeName);
        EMERGENCE_MAPPING_REGISTER_REGULAR (folder);
        EMERGENCE_MAPPING_REGISTRATION_END ();
    }();

    return reflection;
}

const PathMapping::Reflection &PathMapping::Reflect () noexcept
{
    static Reflection reflection = [] ()
    {
        EMERGENCE_MAPPING_REGISTRATION_BEGIN (PathMapping);
        EMERGENCE_MAPPING_REGISTER_REGULAR (configs);
        EMERGENCE_MAPPING_REGISTRATION_END ();
    }();

    return reflection;
}
} // namespace Emergence::Celerity::ResourceConfigPathMappingLoading

#include <atomic>
#include <filesystem>
#include <fstream>

#include <Celerity/Asset/Config/Loading.hpp>
#include <Celerity/Asset/Config/LoadingStateSingleton.hpp>
#include <Celerity/Asset/Config/Messages.hpp>
#include <Celerity/Asset/Config/PathMappingLoading.hpp>
#include <Celerity/PipelineBuilderMacros.hpp>

#include <Log/Log.hpp>

#include <Serialization/Binary.hpp>
#include <Serialization/Yaml.hpp>

#include <SyntaxSugar/Time.hpp>

namespace Emergence::Celerity::AssetConfigLoading
{
const Memory::UniqueString Checkpoint::STARTED {"AssetConfigLoadingStarted"};
const Memory::UniqueString Checkpoint::FINISHED {"AssetConfigLoadingFinished"};

class Loader final : public TaskExecutorBase<Loader>
{
public:
    Loader (TaskConstructor &_constructor,
            std::uint64_t _maxLoadingTimePerFrameNs,
            const Container::Vector<AssetConfigTypeMeta> &_supportedTypes) noexcept;

    void Execute () noexcept;

private:
    struct PerTypeData final
    {
        StandardLayout::Mapping type;
        InsertLongTermQuery insertConfig;
        RemoveValueQuery removeConfig;
        Emergence::Serialization::FieldNameLookupCache cache;
    };

    bool ProcessCurrentRequest (AssetConfigLoadingStateSingleton *_loadingState, std::uint64_t _startTime) noexcept;

    bool ProcessPendingRequests (AssetConfigLoadingStateSingleton *_loadingState) noexcept;

    ModifySingletonQuery modifyLoadingState;
    ModifySequenceQuery modifyRequest;
    ModifySequenceQuery modifyResponse;
    InsertShortTermQuery insertResponse;

    const std::uint64_t maxLoadingTimePerFrameNs;
    Container::Vector<PerTypeData> perTypeData {Memory::Profiler::AllocationGroup::Top ()};

    bool serving = false;
    StandardLayout::Mapping requestedType;
    std::filesystem::recursive_directory_iterator directoryIterator;
};

Loader::Loader (TaskConstructor &_constructor,
                std::uint64_t _maxLoadingTimePerFrameNs,
                const Container::Vector<AssetConfigTypeMeta> &_supportedTypes) noexcept
    : modifyLoadingState (MODIFY_SINGLETON (AssetConfigLoadingStateSingleton)),
      modifyRequest (MODIFY_SEQUENCE (AssetConfigRequest)),
      modifyResponse (MODIFY_SEQUENCE (AssetConfigLoadedResponse)),
      insertResponse (INSERT_SHORT_TERM (AssetConfigLoadedResponse)),
      maxLoadingTimePerFrameNs (_maxLoadingTimePerFrameNs)
{
    _constructor.DependOn (Checkpoint::STARTED);
    _constructor.DependOn (AssetConfigPathMappingLoading::Checkpoint::FINISHED);
    _constructor.MakeDependencyOf (Checkpoint::FINISHED);

    for (const AssetConfigTypeMeta &meta : _supportedTypes)
    {
        perTypeData.emplace_back (PerTypeData {meta.mapping, _constructor.InsertLongTerm (meta.mapping),
                                               _constructor.RemoveValue (meta.mapping, {meta.nameField}),
                                               Serialization::FieldNameLookupCache {meta.mapping}});
    }
}

void Loader::Execute () noexcept
{
    // Clear old responses.
    for (auto responseCursor = modifyResponse.Execute (); *responseCursor; ~responseCursor)
    {
    }

    auto loadingStateCursor = modifyLoadingState.Execute ();
    auto *loadingState = static_cast<AssetConfigLoadingStateSingleton *> (*loadingStateCursor);

    if (!loadingState->pathMappingLoaded)
    {
        return;
    }

    const std::uint64_t startTime = Emergence::Time::NanosecondsSinceStartup ();
    while (ProcessCurrentRequest (loadingState, startTime) && ProcessPendingRequests (loadingState))
    {
    }
}

bool Loader::ProcessCurrentRequest (AssetConfigLoadingStateSingleton *_loadingState, std::uint64_t _startTime) noexcept
{
    constexpr bool CONTINUE_PIPELINE = true;
    constexpr bool STOP_PIPELINE = false;

    if (!serving)
    {
        return CONTINUE_PIPELINE;
    }

    auto perTypeDataIterator = std::find_if (perTypeData.begin (), perTypeData.end (),
                                             [this] (const PerTypeData &_data)
                                             {
                                                 return _data.type == requestedType;
                                             });

    if (perTypeDataIterator == perTypeData.end ())
    {
        EMERGENCE_LOG (ERROR, "AssetConfigLoading: Config type \"", requestedType.GetName (),
                       "\" is not supported! Check loading task registration.");
        return CONTINUE_PIPELINE;
    }

    PerTypeData &perType = *perTypeDataIterator;
    auto stateIterator = std::find_if (_loadingState->typeStates.begin (), _loadingState->typeStates.end (),
                                       [this] (const AssetConfigLoadingStateSingleton::TypeState &_state)
                                       {
                                           return _state.type == requestedType;
                                       });

    if (stateIterator == _loadingState->typeStates.end ())
    {
        EMERGENCE_LOG (ERROR, "AssetConfigLoading: Config type \"", requestedType.GetName (),
                       "\" is not supported! Check path mapping loading task registration and path mapping asset.");
        return CONTINUE_PIPELINE;
    }

    AssetConfigLoadingStateSingleton::TypeState &state = *stateIterator;
    StandardLayout::Field nameField = state.type.GetField (state.nameField);
    assert (nameField);

    while (directoryIterator != std::filesystem::end (directoryIterator))
    {
        if (Emergence::Time::NanosecondsSinceStartup () - _startTime > maxLoadingTimePerFrameNs)
        {
            return STOP_PIPELINE;
        }

        const std::filesystem::directory_entry &entry = *directoryIterator;
        const std::filesystem::path &extension = entry.path ().extension ();

        constexpr const char *const BINARY_SUFFIX = ".bin";
        constexpr const char *const YAML_SUFFIX = ".yaml";

        const bool isBinaryConfig = extension == BINARY_SUFFIX;
        const bool isYamlConfig = extension == YAML_SUFFIX;

        if (isBinaryConfig || isYamlConfig)
        {
            const Container::String relativePath =
                std::filesystem::relative (entry.path (), state.folder)
                    .generic_string<char, std::char_traits<char>, Memory::HeapSTD<char>> ();
            Memory::UniqueString configName;

            if (relativePath.ends_with (BINARY_SUFFIX))
            {
                configName =
                    Memory::UniqueString {relativePath.substr (0u, relativePath.size () - strlen (BINARY_SUFFIX))};
            }
            else if (relativePath.ends_with (YAML_SUFFIX))
            {
                configName =
                    Memory::UniqueString {relativePath.substr (0u, relativePath.size () - strlen (YAML_SUFFIX))};
            }

            // Remove old version of config if it exists.
            {
                auto removeCursor = perType.removeConfig.Execute (&configName);
                if (removeCursor.ReadConst ())
                {
                    ~removeCursor;
                }
            }

            auto insertCursor = perType.insertConfig.Execute ();
            void *config = ++insertCursor;
            bool loaded;

            if (isBinaryConfig)
            {
                std::ifstream input {entry.path (), std::ios::binary};
                loaded = Emergence::Serialization::Binary::DeserializeObject (input, config, requestedType);
            }
            else
            {
                std::ifstream input {entry.path ()};
                loaded = Emergence::Serialization::Yaml::DeserializeObject (input, config, perType.cache);
            }

            if (!loaded)
            {
                EMERGENCE_LOG (ERROR, "AssetConfigLoading: Failed to load config from \"",
                               entry.path ().string ().c_str (), "\"!");
            }

            *static_cast<Memory::UniqueString *> (nameField.GetValue (config)) = configName;
        }

        ++directoryIterator;
    }

    auto insertionCursor = insertResponse.Execute ();
    static_cast<AssetConfigLoadedResponse *> (++insertionCursor)->type = requestedType;
    state.loaded = true;
    return CONTINUE_PIPELINE;
}

bool Loader::ProcessPendingRequests (AssetConfigLoadingStateSingleton *_loadingState) noexcept
{
    serving = false;
    auto requestCursor = modifyRequest.Execute ();

    while (auto *request = static_cast<AssetConfigRequest *> (*requestCursor))
    {
        bool loadingSkipped = false;
        for (const AssetConfigLoadingStateSingleton::TypeState &state : _loadingState->typeStates)
        {
            if (state.type == request->type)
            {
                ~requestCursor;

                if (state.loaded && !request->forceReload)
                {
                    loadingSkipped = true;
                    auto insertionCursor = insertResponse.Execute ();
                    static_cast<AssetConfigLoadedResponse *> (++insertionCursor)->type = state.type;
                    break;
                }

                serving = true;
                requestedType = state.type;
                directoryIterator = std::filesystem::recursive_directory_iterator (state.folder);
                return true;
            }
        }

        if (!loadingSkipped)
        {
            EMERGENCE_LOG (ERROR, "AssetConfigLoading: Unable to find loading state for config type \"",
                           request->type.GetName (), "\". Therefore this type cannot be loaded.");
            ~requestCursor;
        }
    }

    return false;
}

void AddToLoadingPipeline (PipelineBuilder &_builder,
                           std::uint64_t _maxLoadingTimePerFrameNs,
                           const Container::Vector<AssetConfigTypeMeta> &_supportedTypes) noexcept
{
    _builder.AddCheckpoint (Checkpoint::STARTED);
    _builder.AddCheckpoint (Checkpoint::FINISHED);
    _builder.AddTask (Memory::UniqueString {"AssetConfigLoader"})
        .SetExecutor<Loader> (_maxLoadingTimePerFrameNs, _supportedTypes);
}
} // namespace Emergence::Celerity::AssetConfigLoading

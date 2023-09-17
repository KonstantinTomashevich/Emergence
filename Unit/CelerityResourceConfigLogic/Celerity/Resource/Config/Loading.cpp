#include <Celerity/PipelineBuilderMacros.hpp>
#include <Celerity/Resource/Config/Loading.hpp>
#include <Celerity/Resource/Config/LoadingStateSingleton.hpp>
#include <Celerity/Resource/Config/Messages.hpp>
#include <Celerity/WorldSingleton.hpp>

#include <Job/Dispatcher.hpp>

#include <Log/Log.hpp>

namespace Emergence::Celerity::ResourceConfigLoading
{
using namespace Memory::Literals;

const Memory::UniqueString Checkpoint::STARTED {"ResourceConfigLoadingStarted"};
const Memory::UniqueString Checkpoint::FINISHED {"ResourceConfigLoadingFinished"};

class Loader final : public TaskExecutorBase<Loader>
{
public:
    Loader (TaskConstructor &_constructor,
            Resource::Provider::ResourceProvider *_resourceProvider,
            const Container::Vector<ResourceConfigTypeMeta> &_supportedTypes) noexcept;

    void Execute () noexcept;

private:
    struct PerTypeData final
    {
        StandardLayout::Mapping type;
        StandardLayout::FieldId nameField;
        InsertLongTermQuery insertConfig;
        RemoveAscendingRangeQuery removeConfigByAscendingName;
    };

    PerTypeData *FindTypeData (const StandardLayout::Mapping &_mapping) noexcept;

    void ProcessRequests (const WorldSingleton *_world, ResourceConfigLoadingStateSingleton *_loadingState) noexcept;

    void ProcessLoading (const WorldSingleton *_world, ResourceConfigLoadingStateSingleton *_loadingState) noexcept;

    FetchSingletonQuery fetchWorld;
    ModifySingletonQuery modifyLoadingState;

    ModifySequenceQuery modifyRequest;
    ModifySequenceQuery modifyResponse;
    InsertShortTermQuery insertResponse;

    Resource::Provider::ResourceProvider *resourceProvider;
    Container::Vector<PerTypeData> perTypeData {Memory::Profiler::AllocationGroup::Top ()};
};

Loader::Loader (TaskConstructor &_constructor,
                Resource::Provider::ResourceProvider *_resourceProvider,
                const Container::Vector<ResourceConfigTypeMeta> &_supportedTypes) noexcept
    : TaskExecutorBase (_constructor),

      fetchWorld (FETCH_SINGLETON (WorldSingleton)),
      modifyLoadingState (MODIFY_SINGLETON (ResourceConfigLoadingStateSingleton)),

      modifyRequest (MODIFY_SEQUENCE (ResourceConfigRequest)),
      modifyResponse (MODIFY_SEQUENCE (ResourceConfigLoadedResponse)),
      insertResponse (INSERT_SHORT_TERM (ResourceConfigLoadedResponse)),

      resourceProvider (_resourceProvider)
{
    _constructor.DependOn (Checkpoint::STARTED);
    _constructor.MakeDependencyOf (Checkpoint::FINISHED);

    for (const ResourceConfigTypeMeta &meta : _supportedTypes)
    {
        perTypeData.emplace_back (PerTypeData {meta.mapping, meta.nameField, _constructor.InsertLongTerm (meta.mapping),
                                               _constructor.RemoveAscendingRange (meta.mapping, meta.nameField)});
    }
}

void Loader::Execute () noexcept
{
    // Clear old responses.
    for (auto responseCursor = modifyResponse.Execute (); *responseCursor; ~responseCursor)
    {
    }

    auto worldCursor = fetchWorld.Execute ();
    const auto *world = static_cast<const WorldSingleton *> (*worldCursor);

    auto loadingStateCursor = modifyLoadingState.Execute ();
    auto *loadingState = static_cast<ResourceConfigLoadingStateSingleton *> (*loadingStateCursor);

    ProcessRequests (world, loadingState);
    ProcessLoading (world, loadingState);
}

Loader::PerTypeData *Loader::FindTypeData (const StandardLayout::Mapping &_mapping) noexcept
{
    for (PerTypeData &data : perTypeData)
    {
        if (data.type == _mapping)
        {
            return &data;
        }
    }

    return nullptr;
}

void Loader::ProcessRequests (const WorldSingleton *_world, ResourceConfigLoadingStateSingleton *_loadingState) noexcept
{
    if (!_world->contextEscapeAllowed)
    {
        return;
    }

    auto responseCursor = insertResponse.Execute ();
    for (auto requestCursor = modifyRequest.Execute ();
         auto *request = static_cast<ResourceConfigRequest *> (*requestCursor); ~requestCursor)
    {
        if (PerTypeData *data = FindTypeData (request->type))
        {
            bool loaded;
            {
                auto cursor = data->removeConfigByAscendingName.Execute (nullptr, nullptr);
                loaded = cursor.ReadConst ();
            }

            if (loaded && !request->forceReload)
            {
                auto *response = static_cast<ResourceConfigLoadedResponse *> (++responseCursor);
                response->type = request->type;
            }
            else
            {
                bool alreadyLoading = false;
                for (const Handling::Handle<ResourceConfigLoadingSharedState> &sharedState :
                     _loadingState->loadingStates)
                {
                    if (sharedState->configType == request->type)
                    {
                        alreadyLoading = true;
                        break;
                    }
                }

                if (!alreadyLoading)
                {
                    Handling::Handle<ResourceConfigLoadingSharedState> sharedState =
                        _loadingState->loadingStates.emplace_back (
                            new ResourceConfigLoadingSharedState {request->type});
                    sharedState->ReportEscaped (_world);

                    Job::Dispatcher::Global ().Dispatch (
                        Job::Priority::BACKGROUND,
                        [cachedResourceProvider {resourceProvider}, sharedState, mapping {data->type},
                         nameField {data->nameField}] ()
                        {
                            static CPU::Profiler::SectionDefinition loadingSection {*"ResourceConfigLoading"_us,
                                                                                    0xFF999900u};
                            CPU::Profiler::SectionInstance section {loadingSection};

                            for (auto cursor = cachedResourceProvider->FindObjectsByType (sharedState->configType);
                                 **cursor; ++cursor)
                            {
                                void *newConfig = sharedState->configPool.Acquire ();
                                sharedState->configType.Construct (newConfig);

                                switch (
                                    cachedResourceProvider->LoadObject (sharedState->configType, *cursor, newConfig))
                                {
                                case Resource::Provider::LoadingOperationResponse::SUCCESSFUL:
                                    break;

                                case Resource::Provider::LoadingOperationResponse::NOT_FOUND:
                                    EMERGENCE_ASSERT (false);
                                    sharedState->loadingState = ResourceConfigLoadingState::FAILED;
                                    return;

                                case Resource::Provider::LoadingOperationResponse::IO_ERROR:
                                    EMERGENCE_LOG (
                                        ERROR, "ResourceConfigLoading: Encountered IO error while loading config \"",
                                        *cursor, "\" of type \"", sharedState->configType.GetName (), "\".");
                                    sharedState->loadingState = ResourceConfigLoadingState::FAILED;
                                    return;

                                case Resource::Provider::LoadingOperationResponse::WRONG_TYPE:
                                    EMERGENCE_ASSERT (false);
                                    sharedState->loadingState = ResourceConfigLoadingState::FAILED;
                                    return;
                                }

                                *static_cast<Memory::UniqueString *> (
                                    mapping.GetField (nameField).GetValue (newConfig)) = *cursor;
                            }

                            sharedState->loadingState = ResourceConfigLoadingState::SUCCESSFUL;
                        });
                }
            }
        }
        else
        {
            EMERGENCE_LOG (ERROR, "ResourceConfigLoading: Type \"", request->type.GetName (),
                           "\" is not registered as config type!");
        }
    }
}

void Loader::ProcessLoading (const WorldSingleton *_world, ResourceConfigLoadingStateSingleton *_loadingState) noexcept
{
    auto responseCursor = insertResponse.Execute ();
    for (auto iterator = _loadingState->loadingStates.begin (); iterator != _loadingState->loadingStates.end ();)
    {
        const Handling::Handle<ResourceConfigLoadingSharedState> &sharedState = *iterator;
        switch (sharedState->loadingState)
        {
        case ResourceConfigLoadingState::LOADING:
            ++iterator;
            break;

        case ResourceConfigLoadingState::SUCCESSFUL:
            if (!sharedState->configPool.IsEmpty ())
            {
                PerTypeData *data = FindTypeData (sharedState->configType);
                EMERGENCE_ASSERT (data);

                for (auto cursor = data->removeConfigByAscendingName.Execute (nullptr, nullptr); cursor.ReadConst ();)
                {
                    ~cursor;
                }

                auto cursor = data->insertConfig.Execute ();
                for (void *loadedConfig : sharedState->configPool)
                {
                    auto *config = ++cursor;
                    sharedState->configType.MoveConstruct (config, loadedConfig);
                }

                auto *response = static_cast<ResourceConfigLoadedResponse *> (++responseCursor);
                response->type = sharedState->configType;
            }
            else
            {
                EMERGENCE_LOG (ERROR, "ResourceConfigLoading: Unable to find any instances of config type \"",
                               sharedState->configType.GetName (), "\".");
            }

            sharedState->ReportReturned (_world);
            iterator = Container::EraseExchangingWithLast (_loadingState->loadingStates, iterator);
            break;

        case ResourceConfigLoadingState::FAILED:
            EMERGENCE_LOG (ERROR, "ResourceConfigLoading: Failed to load configs of type \"",
                           sharedState->configType.GetName (), "\".");

            sharedState->ReportReturned (_world);
            iterator = Container::EraseExchangingWithLast (_loadingState->loadingStates, iterator);
            break;
        }
    }
}

void AddToLoadingPipeline (PipelineBuilder &_builder,
                           Resource::Provider::ResourceProvider *_resourceProvider,
                           const Container::Vector<ResourceConfigTypeMeta> &_supportedTypes) noexcept
{
    auto visualGroup = _builder.OpenVisualGroup ("ResourceConfigLoading");
    _builder.AddCheckpoint (Checkpoint::STARTED);
    _builder.AddCheckpoint (Checkpoint::FINISHED);
    _builder.AddTask ("ResourceConfigLoader"_us).SetExecutor<Loader> (_resourceProvider, _supportedTypes);
}
} // namespace Emergence::Celerity::ResourceConfigLoading

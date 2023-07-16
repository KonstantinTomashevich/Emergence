#include <Celerity/Assembly/AssemblyDescriptor.hpp>
#include <Celerity/PipelineBuilderMacros.hpp>
#include <Celerity/Resource/Object/Loading.hpp>
#include <Celerity/Resource/Object/LoadingStateSingleton.hpp>
#include <Celerity/Resource/Object/Messages.hpp>

#include <Job/Dispatcher.hpp>

#include <Resource/Object/LibraryLoader.hpp>

namespace Emergence::Celerity::ResourceObjectLoading
{
using namespace Memory::Literals;

const Memory::UniqueString Checkpoint::STARTED {"ResourceObjectLoadingStarted"};
const Memory::UniqueString Checkpoint::FINISHED {"ResourceObjectLoadingFinished"};

class LoadingProcessor final : public TaskExecutorBase<LoadingProcessor>
{
public:
    LoadingProcessor (TaskConstructor &_constructor,
                      Resource::Provider::ResourceProvider *_resourceProvider,
                      Resource::Object::TypeManifest _manifest) noexcept;

    void Execute () noexcept;

private:
    void ProcessRequests (ResourceObjectLoadingStateSingleton *_state) noexcept;

    void ProcessLoading (ResourceObjectLoadingStateSingleton *_state) noexcept;

    ModifySingletonQuery modifyState;

    ModifySequenceQuery modifyRequests;
    InsertShortTermQuery insertResponses;
    ModifySequenceQuery modifyResponses;

    InsertLongTermQuery insertAssemblyDescriptor;
    RemoveValueQuery removeAssemblyDescriptor;

    Resource::Provider::ResourceProvider *resourceProvider;
    Resource::Object::TypeManifest typeManifest;
};

LoadingProcessor::LoadingProcessor (TaskConstructor &_constructor,
                                    Resource::Provider::ResourceProvider *_resourceProvider,
                                    Resource::Object::TypeManifest _manifest) noexcept
    : TaskExecutorBase (_constructor),

      modifyState (MODIFY_SINGLETON (ResourceObjectLoadingStateSingleton)),

      modifyRequests (MODIFY_SEQUENCE (ResourceObjectRequest)),
      insertResponses (INSERT_SHORT_TERM (ResourceObjectLoadedResponse)),
      modifyResponses (MODIFY_SEQUENCE (ResourceObjectLoadedResponse)),

      insertAssemblyDescriptor (INSERT_LONG_TERM (AssemblyDescriptor)),
      removeAssemblyDescriptor (REMOVE_VALUE_1F (AssemblyDescriptor, id)),

      resourceProvider (_resourceProvider),
      typeManifest (std::move (_manifest))
{
    _constructor.DependOn (Checkpoint::STARTED);
    _constructor.MakeDependencyOf (Checkpoint::FINISHED);
}

void LoadingProcessor::Execute () noexcept
{
    for (auto cursor = modifyResponses.Execute (); *cursor; ~cursor)
    {
    }

    auto stateCursor = modifyState.Execute ();
    auto *state = static_cast<ResourceObjectLoadingStateSingleton *> (*stateCursor);
    ProcessRequests (state);
    ProcessLoading (state);
}

void LoadingProcessor::ProcessRequests (ResourceObjectLoadingStateSingleton *_state) noexcept
{
    auto requestCursor = modifyRequests.Execute ();
    if (!*requestCursor)
    {
        return;
    }

    Handling::Handle<ResourceObjectLoadingSharedState> loadingState {
        new ResourceObjectLoadingSharedState (resourceProvider, typeManifest)};

    for (; auto *request = static_cast<ResourceObjectRequest *> (*requestCursor); ~requestCursor)
    {
        if (!request->forceReload)
        {
            if (auto cursor = removeAssemblyDescriptor.Execute (&request->objectId); cursor.ReadConst ())
            {
                // Skip loading of already loaded object.
                continue;
            }
        }

        bool alreadyLoading = false;
        for (const Handling::Handle<ResourceObjectLoadingSharedState> &otherState : _state->sharedStates)
        {
            if (std::find (otherState->requestedObjectList.begin (), otherState->requestedObjectList.end (),
                           request->objectId) != otherState->requestedObjectList.end ())
            {
                alreadyLoading = true;
                break;
            }
        }

        if (alreadyLoading)
        {
            // Already loading as part of other request. Skip.
            continue;
        }

        loadingState->requestedObjectList.emplace_back (request->objectId);
    }

    if (!loadingState->requestedObjectList.empty ())
    {
        Job::Dispatcher::Global ().Dispatch (
            Job::Priority::BACKGROUND,
            [loadingState] ()
            {
                static CPU::Profiler::SectionDefinition loadingSection {*"ResourceObjectLoading"_us, 0xFF999900u};
                CPU::Profiler::SectionInstance section {loadingSection};
                Container::Vector<Resource::Object::LibraryLoadingTask> tasks;

                for (Memory::UniqueString objectId : loadingState->requestedObjectList)
                {
                    tasks.emplace_back () = {objectId};
                }

                loadingState->library = loadingState->libraryLoader.Load (tasks);
                loadingState->loaded.test_and_set (std::memory_order::release);
            });

        _state->sharedStates.emplace_back (std::move (loadingState));
    }
}

void LoadingProcessor::ProcessLoading (ResourceObjectLoadingStateSingleton *_state) noexcept
{
    auto responseCursor = insertResponses.Execute ();
    for (auto iterator = _state->sharedStates.begin (); iterator != _state->sharedStates.end ();)
    {
        const Handling::Handle<ResourceObjectLoadingSharedState> &loadingState = *iterator;
        if (loadingState->loaded.test (std::memory_order::acquire))
        {
            for (const auto &[objectId, objectData] : loadingState->library.GetRegisteredObjectMap ())
            {
                if (objectData.loadedAsParent)
                {
                    continue;
                }

                // If object is already loaded (for example, we're reloading objects), remove it.
                {
                    auto removalCursor = removeAssemblyDescriptor.Execute (&objectId);
                    if (removalCursor.ReadConst ())
                    {
                        ~removalCursor;
                    }
                }

                {
                    auto insertionCursor = insertAssemblyDescriptor.Execute ();
                    auto *descriptor = static_cast<Emergence::Celerity::AssemblyDescriptor *> (++insertionCursor);
                    descriptor->id = objectId;
                    descriptor->components.reserve (objectData.object.changelist.size ());

                    for (const Resource::Object::ObjectComponent &component : objectData.object.changelist)
                    {
                        descriptor->components.emplace_back (component.component);
                    }
                }

                auto *response = static_cast<ResourceObjectLoadedResponse *> (++responseCursor);
                response->objectId = objectId;
            }

            iterator = Container::EraseExchangingWithLast (_state->sharedStates, iterator);
        }
        else
        {
            ++iterator;
        }
    }
}

void AddToLoadingPipeline (PipelineBuilder &_builder,
                           Resource::Provider::ResourceProvider *_resourceProvider,
                           Resource::Object::TypeManifest _typeManifest) noexcept
{
    auto visualGroup = _builder.OpenVisualGroup ("ResourceObjectLoading");
    _builder.AddCheckpoint (Checkpoint::STARTED);
    _builder.AddCheckpoint (Checkpoint::FINISHED);
    _builder.AddTask ("ResourceObjectLoadingProcessor"_us)
        .SetExecutor<LoadingProcessor> (_resourceProvider, std::move (_typeManifest));
}
} // namespace Emergence::Celerity::ResourceObjectLoading

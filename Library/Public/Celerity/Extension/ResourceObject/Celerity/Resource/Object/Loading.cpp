#include <Celerity/Assembly/AssemblyDescriptor.hpp>
#include <Celerity/PipelineBuilderMacros.hpp>
#include <Celerity/Resource/Object/Loading.hpp>
#include <Celerity/Resource/Object/LoadingStateSingleton.hpp>
#include <Celerity/Resource/Object/Messages.hpp>

#include <Resource/Object/LibraryLoader.hpp>

namespace Emergence::Celerity::ResourceObjectLoading
{
using namespace Memory::Literals;

const Memory::UniqueString Checkpoint::STARTED {"ResourceObjectLoadingStarted"};
const Memory::UniqueString Checkpoint::FINISHED {"ResourceObjectLoadingFinished"};

class LoadingProcessor final : public TaskExecutorBase<LoadingProcessor>
{
public:
    LoadingProcessor (TaskConstructor &_constructor, Resource::Object::TypeManifest _manifest) noexcept;

    void Execute () noexcept;

private:
    void ClearResponses () noexcept;

    void FinishLoading (ResourceObjectLoadingStateSingleton *_state) noexcept;

    void ProcessRequests (ResourceObjectLoadingStateSingleton *_state) noexcept;

    ModifySingletonQuery modifyState;

    ModifySequenceQuery modifyObjectRequests;
    ModifySequenceQuery modifyFolderRequests;
    ModifySequenceQuery modifyObjectResponses;
    ModifySequenceQuery modifyFolderResponses;
    RemoveValueQuery removeAssemblyDescriptor;

    InsertShortTermQuery insertObjectResponses;
    InsertShortTermQuery insertFolderResponses;
    InsertLongTermQuery insertAssemblyDescriptor;

    Resource::Object::LibraryLoader libraryLoader;
    bool loadingNow = false;
};

LoadingProcessor::LoadingProcessor (TaskConstructor &_constructor, Resource::Object::TypeManifest _manifest) noexcept
    : modifyState (MODIFY_SINGLETON (ResourceObjectLoadingStateSingleton)),

      modifyObjectRequests (MODIFY_SEQUENCE (ResourceObjectRequest)),
      modifyFolderRequests (MODIFY_SEQUENCE (ResourceObjectFolderRequest)),
      modifyObjectResponses (MODIFY_SEQUENCE (ResourceObjectLoadedResponse)),
      modifyFolderResponses (MODIFY_SEQUENCE (ResourceObjectFolderLoadedResponse)),
      removeAssemblyDescriptor (REMOVE_VALUE_1F (AssemblyDescriptor, id)),

      insertObjectResponses (INSERT_SHORT_TERM (ResourceObjectLoadedResponse)),
      insertFolderResponses (INSERT_SHORT_TERM (ResourceObjectFolderLoadedResponse)),
      insertAssemblyDescriptor (INSERT_LONG_TERM (AssemblyDescriptor)),

      libraryLoader (std::move (_manifest))
{
    _constructor.DependOn (Checkpoint::STARTED);
    _constructor.MakeDependencyOf (Checkpoint::FINISHED);
}

void LoadingProcessor::Execute () noexcept
{
    ClearResponses ();
    auto stateCursor = modifyState.Execute ();
    auto *state = static_cast<ResourceObjectLoadingStateSingleton *> (*stateCursor);

    if (loadingNow && !libraryLoader.IsLoading ())
    {
        FinishLoading (state);
    }

    if (!loadingNow)
    {
        ProcessRequests (state);
    }
}

void LoadingProcessor::ClearResponses () noexcept
{
    for (auto cursor = modifyObjectResponses.Execute (); *cursor; ~cursor)
    {
    }

    for (auto cursor = modifyFolderResponses.Execute (); *cursor; ~cursor)
    {
    }
}

void LoadingProcessor::FinishLoading (ResourceObjectLoadingStateSingleton *_state) noexcept
{
    auto objectResponseCursor = insertObjectResponses.Execute ();
    auto folderResponseCursor = insertFolderResponses.Execute ();

    for (const Resource::Object::LibraryLoadingTask &task : libraryLoader.GetLoadingTasks ())
    {
        if (*task.loadSelectedObject)
        {
            auto *objectResponse = static_cast<ResourceObjectLoadedResponse *> (++objectResponseCursor);
            objectResponse->folder = task.folder;
            objectResponse->object = task.loadSelectedObject;
        }
        else
        {
            auto *folderResponse = static_cast<ResourceObjectFolderLoadedResponse *> (++folderResponseCursor);
            folderResponse->folder = task.folder;
            _state->loadedFolders.emplace (task.folder);
        }
    }

    Resource::Object::Library library = libraryLoader.End ();
    for (const auto &[objectName, objectData] : library.GetRegisteredObjectMap ())
    {
        if (objectData.loadedAsDependency)
        {
            continue;
        }

        // If object is already loaded (for example, we're reloading objects), remove it.
        {
            auto removalCursor = removeAssemblyDescriptor.Execute (&objectName);
            if (removalCursor.ReadConst ())
            {
                ~removalCursor;
            }
        }

        {
            auto insertionCursor = insertAssemblyDescriptor.Execute ();
            auto *descriptor = static_cast<Emergence::Celerity::AssemblyDescriptor *> (++insertionCursor);
            descriptor->id = objectName;
            descriptor->components.reserve (objectData.body.fullChangelist.size ());

            for (const Emergence::StandardLayout::Patch &patch : objectData.body.fullChangelist)
            {
                descriptor->components.emplace_back (patch);
            }
        }
    }

    loadingNow = false;
}

void LoadingProcessor::ProcessRequests (ResourceObjectLoadingStateSingleton *_state) noexcept
{
    auto objectResponseCursor = insertObjectResponses.Execute ();
    auto folderResponseCursor = insertFolderResponses.Execute ();
    Container::Vector<Resource::Object::LibraryLoadingTask> loadingTasks;

    for (auto cursor = modifyObjectRequests.Execute (); auto *request = static_cast<ResourceObjectRequest *> (*cursor);
         ~cursor)
    {
        // Check if object is already loaded.
        auto objectCursor = removeAssemblyDescriptor.Execute (&request->object);

        if (!objectCursor.ReadConst () || request->forceReload)
        {
            loadingTasks.emplace_back () = {request->folder, request->object};
        }
        else
        {
            auto *objectResponse = static_cast<ResourceObjectLoadedResponse *> (++objectResponseCursor);
            objectResponse->folder = request->folder;
            objectResponse->object = request->object;
        }
    }

    for (auto cursor = modifyFolderRequests.Execute ();
         auto *request = static_cast<ResourceObjectFolderRequest *> (*cursor); ~cursor)
    {
        if (_state->loadedFolders.contains (request->folder) && !request->forceReload)
        {
            auto *folderResponse = static_cast<ResourceObjectFolderLoadedResponse *> (++folderResponseCursor);
            folderResponse->folder = request->folder;
        }
        else
        {
            loadingTasks.emplace_back () = {request->folder, ""_us};
        }
    }

    if (!loadingTasks.empty ())
    {
        libraryLoader.Begin (loadingTasks);
        loadingNow = true;
    }
}

void AddToLoadingPipeline (PipelineBuilder &_builder, Resource::Object::TypeManifest _typeManifest) noexcept
{
    auto visualGroup = _builder.OpenVisualGroup ("ResourceObjectLoading");
    _builder.AddCheckpoint (Checkpoint::STARTED);
    _builder.AddCheckpoint (Checkpoint::FINISHED);
    _builder.AddTask ("ResourceObjectLoadingProcessor"_us).SetExecutor<LoadingProcessor> (std::move (_typeManifest));
}
} // namespace Emergence::Celerity::ResourceObjectLoading

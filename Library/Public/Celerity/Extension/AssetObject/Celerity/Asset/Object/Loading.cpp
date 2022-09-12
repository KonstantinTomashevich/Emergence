#include <Asset/Object/LibraryLoader.hpp>

#include <Celerity/Assembly/AssemblyDescriptor.hpp>
#include <Celerity/Asset/Object/Loading.hpp>
#include <Celerity/Asset/Object/LoadingStateSingleton.hpp>
#include <Celerity/Asset/Object/Messages.hpp>
#include <Celerity/PipelineBuilderMacros.hpp>

namespace Emergence::Celerity::AssetObjectLoading
{
using namespace Memory::Literals;

const Memory::UniqueString Checkpoint::STARTED {"AssetObjectLoadingStarted"};
const Memory::UniqueString Checkpoint::FINISHED {"AssetObjectLoadingFinished"};

class LoadingProcessor final : public TaskExecutorBase<LoadingProcessor>
{
public:
    LoadingProcessor (TaskConstructor &_constructor, Asset::Object::TypeManifest _manifest) noexcept;

    void Execute () noexcept;

private:
    void ClearResponses () noexcept;

    void FinishLoading (AssetObjectLoadingStateSingleton *_state) noexcept;

    void ProcessRequests (AssetObjectLoadingStateSingleton *_state) noexcept;

    ModifySingletonQuery modifyState;

    ModifySequenceQuery modifyObjectRequests;
    ModifySequenceQuery modifyFolderRequests;
    ModifySequenceQuery modifyObjectResponses;
    ModifySequenceQuery modifyFolderResponses;
    RemoveValueQuery removeAssemblyDescriptor;

    InsertShortTermQuery insertObjectResponses;
    InsertShortTermQuery insertFolderResponses;
    InsertLongTermQuery insertAssemblyDescriptor;

    Asset::Object::LibraryLoader libraryLoader;
    bool loadingNow = false;
};

LoadingProcessor::LoadingProcessor (TaskConstructor &_constructor, Asset::Object::TypeManifest _manifest) noexcept
    : modifyState (MODIFY_SINGLETON (AssetObjectLoadingStateSingleton)),

      modifyObjectRequests (MODIFY_SEQUENCE (AssetObjectRequest)),
      modifyFolderRequests (MODIFY_SEQUENCE (AssetObjectFolderRequest)),
      modifyObjectResponses (MODIFY_SEQUENCE (AssetObjectLoadedResponse)),
      modifyFolderResponses (MODIFY_SEQUENCE (AssetObjectFolderLoadedResponse)),
      removeAssemblyDescriptor (REMOVE_VALUE_1F (AssemblyDescriptor, id)),

      insertObjectResponses (INSERT_SHORT_TERM (AssetObjectLoadedResponse)),
      insertFolderResponses (INSERT_SHORT_TERM (AssetObjectFolderLoadedResponse)),
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
    auto *state = static_cast<AssetObjectLoadingStateSingleton *> (*stateCursor);

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

void LoadingProcessor::FinishLoading (AssetObjectLoadingStateSingleton *_state) noexcept
{
    auto objectResponseCursor = insertObjectResponses.Execute ();
    auto folderResponseCursor = insertFolderResponses.Execute ();

    for (const Asset::Object::LibraryLoadingTask &task : libraryLoader.GetLoadingTasks ())
    {
        if (*task.loadSelectedObject)
        {
            auto *objectResponse = static_cast<AssetObjectLoadedResponse *> (++objectResponseCursor);
            objectResponse->folder = task.folder;
            objectResponse->object = task.loadSelectedObject;
        }
        else
        {
            auto *folderResponse = static_cast<AssetObjectFolderLoadedResponse *> (++folderResponseCursor);
            folderResponse->folder = task.folder;
            _state->loadedFolders.emplace (task.folder);
        }
    }

    Asset::Object::Library library = libraryLoader.End ();
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

void LoadingProcessor::ProcessRequests (AssetObjectLoadingStateSingleton *_state) noexcept
{
    auto objectResponseCursor = insertObjectResponses.Execute ();
    auto folderResponseCursor = insertFolderResponses.Execute ();
    Container::Vector<Asset::Object::LibraryLoadingTask> loadingTasks;

    for (auto cursor = modifyObjectRequests.Execute (); auto *request = static_cast<AssetObjectRequest *> (*cursor);
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
            auto *objectResponse = static_cast<AssetObjectLoadedResponse *> (++objectResponseCursor);
            objectResponse->folder = request->folder;
            objectResponse->object = request->object;
        }
    }

    for (auto cursor = modifyFolderRequests.Execute ();
         auto *request = static_cast<AssetObjectFolderRequest *> (*cursor); ~cursor)
    {
        if (_state->loadedFolders.contains (request->folder) && !request->forceReload)
        {
            auto *folderResponse = static_cast<AssetObjectFolderLoadedResponse *> (++folderResponseCursor);
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

void AddToLoadingPipeline (PipelineBuilder &_builder, Asset::Object::TypeManifest _typeManifest) noexcept
{
    _builder.AddCheckpoint (Checkpoint::STARTED);
    _builder.AddCheckpoint (Checkpoint::FINISHED);
    _builder.AddTask ("AssetObjectLoadingProcessor"_us).SetExecutor<LoadingProcessor> (std::move (_typeManifest));
}
} // namespace Emergence::Celerity::AssetObjectLoading

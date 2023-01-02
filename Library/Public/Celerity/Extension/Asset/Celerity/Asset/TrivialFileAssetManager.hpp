#pragma once

#include <filesystem>

#include <Celerity/Asset/Asset.hpp>
#include <Celerity/Asset/AssetFileLoadingState.hpp>
#include <Celerity/Asset/AssetManagement.hpp>
#include <Celerity/Asset/Events.hpp>
#include <Celerity/PipelineBuilderMacros.hpp>

#include <Log/Log.hpp>

#include <Celerity/PipelineBuilder.hpp>
#include <SyntaxSugar/Time.hpp>

namespace Emergence::Celerity
{
/// \brief Asset manager implementation for assets that are just loaded from drive and initialized afterwards.
/// \details Asset type specific logic is done through CRTP. The successor class must:
///          * Have `using AssetType = YourAssetType;` declaration.
///          * Have `constexpr static const char *const LOG_NAME = "YourLogName";` declaration.
///            For example, TextureManagement or FontManagement.
///          * Have `<any convertable to string type> ExtractFilePath (Memory::UniqueString _assetId) noexcept.` method.
///          * Have `AssetState FinishLoading (AssetFileLoadingState *_loadingState) noexcept;` method.
template <typename Successor>
class TrivialFileAssetManager
{
public:
    /// \brief Constructs new manager instance.
    ///
    /// \param _constructor Task constructor used to create queries and establish graph dependencies.
    /// \param _rootPaths Paths to directories where assets of required type might be stored.
    /// \param _maxLoadingTimePerFrameNs Maximum time per frame allocated for Font loading.
    /// \param _stateUpdateEvent Type of event used to sent state update info.
    TrivialFileAssetManager (TaskConstructor &_constructor,
                             const Container::Vector<Memory::UniqueString> &_rootPaths,
                             uint64_t _maxLoadingTimePerFrameNs,
                             const StandardLayout::Mapping &_stateUpdateEvent) noexcept;

    void Execute () noexcept;

private:
    static const AssetFileLoadingState::Reflection &InstancedAssetFileLoadingStateReflection () noexcept;

    void ProcessLoading () noexcept;

    AssetState StartLoading (Memory::UniqueString _assetId) noexcept;

    static AssetState ContinueLoadingData (AssetFileLoadingState *_loadingState) noexcept;

    void ProcessUnloading () noexcept;

    void Unload (Memory::UniqueString _assetId) noexcept;

    InsertShortTermQuery insertAssetStateEvent;
    FetchSequenceQuery fetchAssetRemovedEvents;
    FetchValueQuery fetchAssetByTypeNumberAndState;

    InsertLongTermQuery insertAssetFileLoadingState;
    RemoveValueQuery removeAssetById;
    ModifyValueQuery modifyAssetFileLoadingStateById;

    Container::Vector<Memory::UniqueString> rootPaths {Memory::Profiler::AllocationGroup::Top ()};
    const uint64_t maxLoadingTimePerFrameNs;
};

template <typename Successor>
TrivialFileAssetManager<Successor>::TrivialFileAssetManager (
    TaskConstructor &_constructor,
    const Container::Vector<Emergence::Memory::UniqueString> &_rootPaths,
    uint64_t _maxLoadingTimePerFrameNs,
    const StandardLayout::Mapping &_stateUpdateEvent) noexcept
    : insertAssetStateEvent (_constructor.InsertShortTerm (_stateUpdateEvent)),
      fetchAssetRemovedEvents (FETCH_SEQUENCE (AssetRemovedNormalEvent)),
      fetchAssetByTypeNumberAndState (FETCH_VALUE_2F (Asset, typeNumber, state)),

      insertAssetFileLoadingState (_constructor.InsertLongTerm (InstancedAssetFileLoadingStateReflection ().mapping)),
      removeAssetById (REMOVE_VALUE_1F (Successor::AssetType, assetId)),
      modifyAssetFileLoadingStateById (_constructor.ModifyValue (
          InstancedAssetFileLoadingStateReflection ().mapping, {InstancedAssetFileLoadingStateReflection ().assetId})),

      maxLoadingTimePerFrameNs (_maxLoadingTimePerFrameNs)
{
    rootPaths.reserve (_rootPaths.size ());
    for (Memory::UniqueString root : _rootPaths)
    {
        rootPaths.emplace_back (root);
    }

    _constructor.DependOn (AssetManagement::Checkpoint::ASSET_LOADING_STARTED);
    _constructor.MakeDependencyOf (AssetManagement::Checkpoint::ASSET_LOADING_FINISHED);
}

template <typename Successor>
void TrivialFileAssetManager<Successor>::Execute () noexcept
{
    ProcessLoading ();
    ProcessUnloading ();
}

template <typename Successor>
const AssetFileLoadingState::Reflection &
TrivialFileAssetManager<Successor>::InstancedAssetFileLoadingStateReflection () noexcept
{
    static AssetFileLoadingState::Reflection reflection =
        AssetFileLoadingState::InstancedReflect (Memory::UniqueString {
            EMERGENCE_BUILD_STRING (Successor::AssetType::Reflect ().mapping.GetName (), "AssetFileLoadingState")});
    return reflection;
}

template <typename Successor>
void TrivialFileAssetManager<Successor>::ProcessLoading () noexcept
{
    struct
    {
        StandardLayout::Mapping mapping = Successor::AssetType::Reflect ().mapping;
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

        AssetState newState = AssetState::LOADING;
        bool needsInitialization;

        {
            auto loadingStateCursor = modifyAssetFileLoadingStateById.Execute (&asset->id);
            needsInitialization = !*loadingStateCursor;
        }

        if (needsInitialization)
        {
            Unload (asset->id);
            newState = StartLoading (asset->id);
        }

        auto loadingStateCursor = modifyAssetFileLoadingStateById.Execute (&asset->id);
        auto *loadingState = static_cast<AssetFileLoadingState *> (*loadingStateCursor);

        while (newState == AssetState::LOADING &&
               Emergence::Time::NanosecondsSinceStartup () - startTime < maxLoadingTimePerFrameNs)
        {
            newState = ContinueLoadingData (loadingState);
        }

        if (newState != AssetState::LOADING)
        {
            if (newState == AssetState::READY)
            {
                newState = static_cast<Successor *> (this)->FinishLoading (loadingState);
            }

            auto eventCursor = insertAssetStateEvent.Execute ();
            auto *event = static_cast<AssetStateUpdateEventView *> (++eventCursor);
            event->assetId = asset->id;
            event->state = newState;

            if (loadingState)
            {
                ~loadingStateCursor;
            }
        }
    }
}

template <typename Successor>
AssetState TrivialFileAssetManager<Successor>::StartLoading (Memory::UniqueString _assetId) noexcept
{
    for (Memory::UniqueString root : rootPaths)
    {
        const std::filesystem::path path =
            EMERGENCE_BUILD_STRING (root, "/", static_cast<Successor *> (this)->ExtractFilePath (_assetId));

        if (std::filesystem::exists (path))
        {
            Container::String pathString = path.generic_string<char, std::char_traits<char>, Memory::HeapSTD<char>> ();
            auto cursor = insertAssetFileLoadingState.Execute ();
            auto *state = static_cast<AssetFileLoadingState *> (++cursor);
            state->assetId = _assetId;

            AssetFileLoadingStatus fileLoadingState = state->StartLoading (pathString.c_str ());
            EMERGENCE_ASSERT (fileLoadingState != AssetFileLoadingStatus::FINISHED);

            if (fileLoadingState == AssetFileLoadingStatus::FAILED)
            {
                EMERGENCE_LOG (ERROR, Successor::LOG_NAME, ": Unable to open asset file \"", pathString, "\".");
                return AssetState::CORRUPTED;
            }

            EMERGENCE_ASSERT (fileLoadingState == AssetFileLoadingStatus::LOADING);
            return AssetState::LOADING;
        }
    }

    return AssetState::MISSING;
}

template <typename Successor>
AssetState TrivialFileAssetManager<Successor>::ContinueLoadingData (AssetFileLoadingState *_loadingState) noexcept
{
    static constexpr uint32_t CHUNK_SIZE = 16u * 1024u;
    switch (_loadingState->ContinueLoading (CHUNK_SIZE))
    {
    case AssetFileLoadingStatus::LOADING:
        return AssetState::LOADING;

    case AssetFileLoadingStatus::FAILED:
        EMERGENCE_LOG (ERROR, Successor::LOG_NAME, ": Encountered IO error while reading asset \"",
                       _loadingState->assetId, "\" data.");
        return AssetState::CORRUPTED;

    case AssetFileLoadingStatus::FINISHED:
        return AssetState::READY;
    }

    EMERGENCE_ASSERT (false);
    return AssetState::CORRUPTED;
}

template <typename Successor>
void TrivialFileAssetManager<Successor>::ProcessUnloading () noexcept
{
    for (auto eventCursor = fetchAssetRemovedEvents.Execute ();
         const auto *event = static_cast<const AssetRemovedNormalEvent *> (*eventCursor); ++eventCursor)
    {
        Unload (event->id);
    }
}

template <typename Successor>
void TrivialFileAssetManager<Successor>::Unload (Memory::UniqueString _assetId) noexcept
{
    if (auto fileLoadingStateCursor = modifyAssetFileLoadingStateById.Execute (&_assetId); *fileLoadingStateCursor)
    {
        ~fileLoadingStateCursor;
    }

    if (auto assetCursor = removeAssetById.Execute (&_assetId); assetCursor.ReadConst ())
    {
        ~assetCursor;
    }
}
} // namespace Emergence::Celerity

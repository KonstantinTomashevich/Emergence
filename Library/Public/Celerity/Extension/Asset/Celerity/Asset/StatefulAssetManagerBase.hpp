#pragma once

#include <filesystem>

#include <Celerity/Asset/Asset.hpp>
#include <Celerity/Asset/AssetManagement.hpp>
#include <Celerity/Asset/Events.hpp>
#include <Celerity/PipelineBuilderMacros.hpp>

#include <Log/Log.hpp>

#include <Celerity/PipelineBuilder.hpp>
#include <SyntaxSugar/Time.hpp>

namespace Emergence::Celerity
{
/// \brief Asset manager implementation for assets that use special state object
///        during loading in order to make use of background job dispatch.
/// \details Background job dispatch usage requires careful design, because raw pointer
///          to state object escapes Celerity context. This class carefully works with
///          state object to make sure it is not deleted until background job finishes
///          with some AssetState.
///
///          Asset type specific logic is done through CRTP. The successor class must:
///          * Have `using AssetType = YourAssetType;` declaration.
///          * Have `using LoadingState = YourLoadingState;` declaration.
///          * Have `AssetState StartLoading (LoadingState *_loadingState) noexcept;` method.
///          * Have `AssetState TryFinishLoading (LoadingState *_loadingState) noexcept;` method.
template <typename Successor>
class StatefulAssetManagerBase
{
public:
    StatefulAssetManagerBase (TaskConstructor &_constructor, const StandardLayout::Mapping &_stateUpdateEvent) noexcept;

    void Execute () noexcept;

protected:
    void InvalidateLoadingState (Memory::UniqueString _assetId) noexcept;

private:
    void ProcessLoading () noexcept;

    void ProcessUnloading () noexcept;

    InsertShortTermQuery insertAssetStateEvent;
    FetchSequenceQuery fetchAssetRemovedEvents;
    FetchValueQuery fetchAssetByTypeNumberAndState;

    InsertLongTermQuery insertLoadingState;
    ModifyValueQuery modifyLoadingStateById;
    RemoveSignalQuery removeInvalidLoadingState;
};

template <typename Successor>
StatefulAssetManagerBase<Successor>::StatefulAssetManagerBase (
    TaskConstructor &_constructor, const StandardLayout::Mapping &_stateUpdateEvent) noexcept
    : insertAssetStateEvent (_constructor.InsertShortTerm (_stateUpdateEvent)),
      fetchAssetRemovedEvents (FETCH_SEQUENCE (AssetRemovedNormalEvent)),
      fetchAssetByTypeNumberAndState (FETCH_VALUE_2F (Asset, typeNumber, state)),

      insertLoadingState (INSERT_LONG_TERM (Successor::LoadingState)),
      modifyLoadingStateById (MODIFY_VALUE_1F (Successor::LoadingState, assetId)),
      removeInvalidLoadingState (REMOVE_SIGNAL (Successor::LoadingState, valid, false))
{
    _constructor.DependOn (AssetManagement::Checkpoint::ASSET_LOADING_STARTED);
    _constructor.MakeDependencyOf (AssetManagement::Checkpoint::ASSET_LOADING_FINISHED);
}

template <typename Successor>
void StatefulAssetManagerBase<Successor>::Execute () noexcept
{
    ProcessLoading ();
    ProcessUnloading ();
}

template <typename Successor>
void StatefulAssetManagerBase<Successor>::InvalidateLoadingState (Memory::UniqueString _assetId) noexcept
{
    if (auto loadingStateCursor = modifyLoadingStateById.Execute (&_assetId);
        auto *loadingState = static_cast<typename Successor::LoadingState *> (*loadingStateCursor))
    {
        loadingState->valid = false;
    }
}

template <typename Successor>
void StatefulAssetManagerBase<Successor>::ProcessLoading () noexcept
{
    struct
    {
        StandardLayout::Mapping mapping = Successor::AssetType::Reflect ().mapping;
        AssetState state = AssetState::LOADING;
    } loadingMaterialsParameter;

    for (auto assetCursor = fetchAssetByTypeNumberAndState.Execute (&loadingMaterialsParameter);
         const auto *asset = static_cast<const Asset *> (*assetCursor); ++assetCursor)
    {
        AssetState state = AssetState::LOADING;
        bool needsInitialization = true;

        if (auto loadingStateCursor = modifyLoadingStateById.Execute (&asset->id);
            auto *loadingState = static_cast<typename Successor::LoadingState *> (*loadingStateCursor))
        {
            needsInitialization = false;
            if (!loadingState->valid)
            {
                // We cannot do anything until invalidated state can be safely removed.
                continue;
            }

            if (loadingState->state != AssetState::LOADING)
            {
                state = static_cast<Successor *> (this)->TryFinishLoading (loadingState);
                if (state != AssetState::LOADING)
                {
                    ~loadingStateCursor;
                }
            }
        }

        if (needsInitialization)
        {
            // If we're reloading material, unload old one first.
            static_cast<Successor *> (this)->Unload (asset->id);

            auto loadingStateCursor = insertLoadingState.Execute ();
            auto *loadingState = static_cast<typename Successor::LoadingState *> (++loadingStateCursor);
            loadingState->assetId = asset->id;

            state = static_cast<Successor *> (this)->StartLoading (loadingState);
        }

        if (state != AssetState::LOADING)
        {
            auto eventCursor = insertAssetStateEvent.Execute ();
            auto *event = static_cast<AssetStateUpdateEventView *> (++eventCursor);
            event->assetId = asset->id;
            event->state = state;
        }
    }
}

template <typename Successor>
void StatefulAssetManagerBase<Successor>::ProcessUnloading () noexcept
{
    for (auto eventCursor = fetchAssetRemovedEvents.Execute ();
         const auto *event = static_cast<const AssetRemovedNormalEvent *> (*eventCursor); ++eventCursor)
    {
        static_cast<Successor *> (this)->Unload (event->id);
    }

    for (auto loadingStateCursor = removeInvalidLoadingState.Execute ();
         const auto *loadingState =
             static_cast<const typename Successor::LoadingState *> (loadingStateCursor.ReadConst ());)
    {
        if (loadingState->state != AssetState::LOADING)
        {
            ~loadingStateCursor;
        }
        else
        {
            ++loadingStateCursor;
        }
    }
}
} // namespace Emergence::Celerity

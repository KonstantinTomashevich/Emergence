#pragma once

#include <Celerity/Asset/Asset.hpp>
#include <Celerity/Asset/AssetManagement.hpp>
#include <Celerity/Asset/Events.hpp>
#include <Celerity/PipelineBuilder.hpp>
#include <Celerity/PipelineBuilderMacros.hpp>
#include <Celerity/WorldSingleton.hpp>

#include <Log/Log.hpp>

namespace Emergence::Celerity
{
/// \brief Syntax sugar for implementing asset managers that use separate loading state.
/// \details This class provides simple loading routine for managers that rely on background
///          loading processing through context scape routine with finalization.
///
///          Asset type specific logic is done through CRTP. The successor class must:
///          * Have `using AssetType = YourAssetType;` declaration.
///          * Have `using LoadingState = YourLoadingState;` declaration.
///            Loading state must have `std::atomic<AssetState> assetType` field.
///          * Have `AssetState StartLoading (LoadingState *_loadingState) noexcept;` method.
///          * Have `AssetState TryFinishLoading (LoadingState *_loadingState) noexcept;` method.
template <typename Successor>
class StatefulAssetManagerBase
{
public:
    StatefulAssetManagerBase (TaskConstructor &_constructor, const StandardLayout::Mapping &_stateUpdateEvent) noexcept;

    void Execute () noexcept;

private:
    void ProcessLoading () noexcept;

    void ProcessUnloading () noexcept;

    FetchSingletonQuery fetchWorld;

    InsertShortTermQuery insertAssetStateEvent;
    FetchSequenceQuery fetchAssetRemovedEvents;
    FetchValueQuery fetchAssetByTypeNumberAndState;

    InsertLongTermQuery insertLoadingState;
    ModifyValueQuery modifyLoadingStateById;
    RemoveValueQuery removeLoadingStateById;
};

template <typename Successor>
StatefulAssetManagerBase<Successor>::StatefulAssetManagerBase (
    TaskConstructor &_constructor, const StandardLayout::Mapping &_stateUpdateEvent) noexcept
    : fetchWorld (FETCH_SINGLETON (WorldSingleton)),

      insertAssetStateEvent (_constructor.InsertShortTerm (_stateUpdateEvent)),
      fetchAssetRemovedEvents (FETCH_SEQUENCE (AssetRemovedNormalEvent)),
      fetchAssetByTypeNumberAndState (FETCH_VALUE_2F (Asset, typeNumber, state)),

      insertLoadingState (INSERT_LONG_TERM (Successor::LoadingState)),
      modifyLoadingStateById (MODIFY_VALUE_1F (Successor::LoadingState, assetId)),
      removeLoadingStateById (REMOVE_VALUE_1F (Successor::LoadingState, assetId))
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
void StatefulAssetManagerBase<Successor>::ProcessLoading () noexcept
{
    auto worldCursor = fetchWorld.Execute ();
    const auto *world = static_cast<const WorldSingleton *> (*worldCursor);

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
            if (loadingState->sharedState->state != AssetState::LOADING)
            {
                state = static_cast<Successor *> (this)->TryFinishLoading (loadingState);
                if (state != AssetState::LOADING)
                {
                    // We're reporting the return only after everything is loaded, because
                    // we're usually unable to correctly reload shared state in case of hot reload.
                    loadingState->sharedState->ReportReturned (world);

                    ~loadingStateCursor;
                }
            }
        }

        if (needsInitialization && world->contextEscapeAllowed)
        {
            {
                // If we're reloading the asset, unload old one first.
                static_cast<Successor *> (this)->Unload (asset->id);

                auto loadingStateCursor = insertLoadingState.Execute ();
                auto *loadingState = static_cast<typename Successor::LoadingState *> (++loadingStateCursor);
                loadingState->assetId = asset->id;

                loadingState->sharedState->ReportEscaped (world);
                state = static_cast<Successor *> (this)->StartLoading (loadingState);
            }

            // If we have failed to start loading, remove the loading state.
            if (state != AssetState::LOADING)
            {
                auto loadingStateCursor = removeLoadingStateById.Execute (&asset->id);
                EMERGENCE_ASSERT (loadingStateCursor.ReadConst ());
                const auto *loadingState =
                    static_cast<const typename Successor::LoadingState *> (loadingStateCursor.ReadConst ());

                loadingState->sharedState->ReportReturned (world);
                ~loadingStateCursor;
            }
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
        if (auto loadingStateCursor = removeLoadingStateById.Execute (&event->id); loadingStateCursor.ReadConst ())
        {
            ~loadingStateCursor;
        }
    }
}
} // namespace Emergence::Celerity

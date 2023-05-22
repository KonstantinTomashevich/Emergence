#include <Celerity/Asset/AssetManagement.hpp>
#include <Celerity/Asset/Events.hpp>
#include <Celerity/Asset/Render/2d/Sprite2dUvAnimation.hpp>
#include <Celerity/Asset/Render/2d/Sprite2dUvAnimationLoadingState.hpp>
#include <Celerity/Asset/Render/2d/Sprite2dUvAnimationManagement.hpp>
#include <Celerity/Asset/StatefulAssetManagerBase.hpp>
#include <Celerity/PipelineBuilderMacros.hpp>
#include <Celerity/Render/2d/Sprite2dUvAnimation.hpp>

#include <Job/Dispatcher.hpp>

#include <Log/Log.hpp>

#include <Render/Backend/Configuration.hpp>

namespace Emergence::Celerity::Sprite2dUvAnimationManagement
{
class Manager final : public TaskExecutorBase<Manager>, public StatefulAssetManagerBase<Manager>
{
public:
    using AssetType = Sprite2dUvAnimation;

    using LoadingState = Sprite2dUvAnimationLoadingState;

    Manager (TaskConstructor &_constructor,
             ResourceProvider::ResourceProvider *_resourceProvider,
             const StandardLayout::Mapping &_stateUpdateEvent) noexcept;

private:
    friend class StatefulAssetManagerBase<Manager>;

    AssetState StartLoading (Sprite2dUvAnimationLoadingState *_loadingState) noexcept;

    AssetState TryFinishLoading (Sprite2dUvAnimationLoadingState *_loadingState) noexcept;

    void Unload (Memory::UniqueString _assetId) noexcept;

    InsertLongTermQuery insertAnimation;
    RemoveValueQuery removeAnimationById;

    ResourceProvider::ResourceProvider *resourceProvider;
};

Manager::Manager (TaskConstructor &_constructor,
                  ResourceProvider::ResourceProvider *_resourceProvider,
                  const StandardLayout::Mapping &_stateUpdateEvent) noexcept
    : StatefulAssetManagerBase (_constructor, _stateUpdateEvent),

      insertAnimation (INSERT_LONG_TERM (Sprite2dUvAnimation)),
      removeAnimationById (REMOVE_VALUE_1F (Sprite2dUvAnimation, assetId)),

      resourceProvider (_resourceProvider)
{
    _constructor.DependOn (AssetManagement::Checkpoint::ASSET_LOADING_STARTED);
    _constructor.MakeDependencyOf (AssetManagement::Checkpoint::ASSET_LOADING_FINISHED);
}

AssetState Manager::StartLoading (Sprite2dUvAnimationLoadingState *_loadingState) noexcept
{
    Job::Dispatcher::Global ().Dispatch (
        Job::Priority::BACKGROUND,
        [assetId {_loadingState->assetId}, cachedResourceProvider {resourceProvider},
         sharedState {_loadingState->sharedState}] ()
        {
            switch (cachedResourceProvider->LoadObject (Sprite2dUvAnimationAsset::Reflect ().mapping, assetId,
                                                        &sharedState->asset))
            {
            case ResourceProvider::LoadingOperationResponse::SUCCESSFUL:
                sharedState->state = AssetState::READY;
                break;

            case ResourceProvider::LoadingOperationResponse::NOT_FOUND:
                EMERGENCE_LOG (ERROR, "Sprite2dUvAnimationManagement: Unable to find animation \"", assetId, "\".");
                sharedState->state = AssetState::MISSING;
                break;

            case ResourceProvider::LoadingOperationResponse::IO_ERROR:
                EMERGENCE_LOG (ERROR, "Sprite2dUvAnimationManagement: Failed to read animation \"", assetId, "\".");
                sharedState->state = AssetState::CORRUPTED;
                break;

            case ResourceProvider::LoadingOperationResponse::WRONG_TYPE:
                EMERGENCE_LOG (ERROR, "Sprite2dUvAnimationManagement: Object \"", assetId, "\" is not an animation.");
                sharedState->state = AssetState::CORRUPTED;
                break;
            }
        });

    return AssetState::LOADING;
}

AssetState Manager::TryFinishLoading (Sprite2dUvAnimationLoadingState *_loadingState) noexcept
{
    auto animationCursor = insertAnimation.Execute ();
    auto *animation = static_cast<Sprite2dUvAnimation *> (++animationCursor);
    animation->assetId = _loadingState->assetId;
    animation->materialInstanceId = _loadingState->sharedState->asset.materialInstanceId;

    for (const Sprite2dUvAnimationFrameInfo &info : _loadingState->sharedState->asset.frames)
    {
        animation->frames.emplace_back () = {info.uv, static_cast<uint64_t> (info.durationS * 1e9f), 0u};
    }

    for (size_t index = 1u; index < animation->frames.size (); ++index)
    {
        animation->frames[index].startTimeNs =
            animation->frames[index - 1u].startTimeNs + animation->frames[index - 1u].durationNs;
    }

    animation->frames.shrink_to_fit ();
    return AssetState::READY;
}

void Manager::Unload (Memory::UniqueString _assetId) noexcept
{
    if (auto animationCursor = removeAnimationById.Execute (&_assetId); animationCursor.ReadConst ())
    {
        ~animationCursor;
    }
}

void AddToNormalUpdate (PipelineBuilder &_pipelineBuilder,
                        ResourceProvider::ResourceProvider *_resourceProvider,
                        const AssetReferenceBindingEventMap &_eventMap) noexcept
{
    auto iterator = _eventMap.stateUpdate.find (Sprite2dUvAnimation::Reflect ().mapping);
    if (iterator == _eventMap.stateUpdate.end ())
    {
        EMERGENCE_LOG (WARNING,
                       "Sprite2dUvAnimationManagement: Task not registered, because Sprite2dUvAnimation is not found "
                       "in state update map. Perhaps it is not referenced by anything?");
        return;
    }

    auto visualGroup = _pipelineBuilder.OpenVisualGroup ("Sprite2dUvAnimationManagement");
    _pipelineBuilder.AddTask (Memory::UniqueString {"Sprite2dUvAnimationManager"})
        .SetExecutor<Manager> (_resourceProvider, iterator->second);
}
} // namespace Emergence::Celerity::Sprite2dUvAnimationManagement

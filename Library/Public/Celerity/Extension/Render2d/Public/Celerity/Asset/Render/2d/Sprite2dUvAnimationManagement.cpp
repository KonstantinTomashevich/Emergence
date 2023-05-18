#include <filesystem>
#include <fstream>

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

#include <Serialization/Binary.hpp>
#include <Serialization/Yaml.hpp>

namespace Emergence::Celerity::Sprite2dUvAnimationManagement
{
class Manager final : public TaskExecutorBase<Manager>, public StatefulAssetManagerBase<Manager>
{
public:
    using AssetType = Sprite2dUvAnimation;

    using LoadingState = Sprite2dUvAnimationLoadingState;

    Manager (TaskConstructor &_constructor,
             const Container::Vector<Memory::UniqueString> &_animationRootPaths,
             const StandardLayout::Mapping &_stateUpdateEvent) noexcept;

private:
    friend class StatefulAssetManagerBase<Manager>;

    AssetState StartLoading (Sprite2dUvAnimationLoadingState *_loadingState) noexcept;

    AssetState TryFinishLoading (Sprite2dUvAnimationLoadingState *_loadingState) noexcept;

    void Unload (Memory::UniqueString _assetId) noexcept;

    InsertLongTermQuery insertAnimation;
    RemoveValueQuery removeAnimationById;

    Container::Vector<Memory::UniqueString> animationRootPaths {Memory::Profiler::AllocationGroup::Top ()};
};

Manager::Manager (TaskConstructor &_constructor,
                  const Container::Vector<Memory::UniqueString> &_animationRootPaths,
                  const StandardLayout::Mapping &_stateUpdateEvent) noexcept
    : StatefulAssetManagerBase (_constructor, _stateUpdateEvent),

      insertAnimation (INSERT_LONG_TERM (Sprite2dUvAnimation)),
      removeAnimationById (REMOVE_VALUE_1F (Sprite2dUvAnimation, assetId))
{
    animationRootPaths.reserve (_animationRootPaths.size ());
    for (Memory::UniqueString animationRoot : _animationRootPaths)
    {
        animationRootPaths.emplace_back (animationRoot);
    }

    _constructor.DependOn (AssetManagement::Checkpoint::ASSET_LOADING_STARTED);
    _constructor.MakeDependencyOf (AssetManagement::Checkpoint::ASSET_LOADING_FINISHED);
}

AssetState Manager::StartLoading (Sprite2dUvAnimationLoadingState *_loadingState) noexcept
{
    for (Memory::UniqueString root : animationRootPaths)
    {
        std::filesystem::path binaryPath = EMERGENCE_BUILD_STRING (root, "/", _loadingState->assetId, ".animation.bin");
        if (std::filesystem::exists (binaryPath))
        {
            Job::Dispatcher::Global ().Dispatch (
                Job::Priority::BACKGROUND,
                [sharedState {_loadingState->sharedState}, binaryPath] ()
                {
                    std::ifstream input {binaryPath, std::ios::binary};
                    if (!Serialization::Binary::DeserializeObject (input, &sharedState->asset,
                                                                   Sprite2dUvAnimationAsset::Reflect ().mapping, {}))
                    {
                        EMERGENCE_LOG (
                            ERROR, "Sprite2dUvAnimationManagement: Unable to load animation from \"",
                            binaryPath.generic_string<char, std::char_traits<char>, Memory::HeapSTD<char>> (), "\".");

                        sharedState->state = AssetState::CORRUPTED;
                    }
                    else
                    {
                        sharedState->state = AssetState::READY;
                    }
                });

            return AssetState::LOADING;
        }

        std::filesystem::path yamlPath = EMERGENCE_BUILD_STRING (root, "/", _loadingState->assetId, ".animation.yaml");
        if (std::filesystem::exists (yamlPath))
        {
            Job::Dispatcher::Global ().Dispatch (
                Job::Priority::BACKGROUND,
                [sharedState {_loadingState->sharedState}, yamlPath] ()
                {
                    std::ifstream input {yamlPath};
                    if (!Serialization::Yaml::DeserializeObject (input, &sharedState->asset,
                                                                 Sprite2dUvAnimationAsset::Reflect ().mapping, {}))
                    {
                        EMERGENCE_LOG (ERROR, "Sprite2dUvAnimationManagement: Unable to load animation from \"",
                                       yamlPath.generic_string<char, std::char_traits<char>, Memory::HeapSTD<char>> (),
                                       "\".");

                        sharedState->state = AssetState::CORRUPTED;
                    }
                    else
                    {
                        sharedState->state = AssetState::READY;
                    }
                });

            return AssetState::LOADING;
        }
    }

    EMERGENCE_LOG (ERROR, "Sprite2dUvAnimationManagement: Unable to find animation \"", _loadingState->assetId, "\".");
    return AssetState::MISSING;
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
                        const Container::Vector<Memory::UniqueString> &_animationRootPaths,
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
        .SetExecutor<Manager> (_animationRootPaths, iterator->second);
}
} // namespace Emergence::Celerity::Sprite2dUvAnimationManagement

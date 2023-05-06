#include <filesystem>
#include <fstream>

#include <Celerity/Asset/AssetManagement.hpp>
#include <Celerity/Asset/Events.hpp>
#include <Celerity/Asset/Render/2d/Sprite2dUvAnimation.hpp>
#include <Celerity/Asset/Render/2d/Sprite2dUvAnimationManagement.hpp>
#include <Celerity/PipelineBuilderMacros.hpp>
#include <Celerity/Render/2d/Sprite2dUvAnimation.hpp>

#include <Log/Log.hpp>

#include <Render/Backend/Configuration.hpp>

#include <Serialization/Binary.hpp>
#include <Serialization/Yaml.hpp>

#include <SyntaxSugar/Time.hpp>

namespace Emergence::Celerity::Sprite2dUvAnimationManagement
{
class Manager final : public TaskExecutorBase<Manager>
{
public:
    Manager (TaskConstructor &_constructor,
             const Container::Vector<Memory::UniqueString> &_animationRootPaths,
             uint64_t _maxLoadingTimePerFrameNs,
             const StandardLayout::Mapping &_stateUpdateEvent) noexcept;

    void Execute () noexcept;

private:
    void ProcessLoading () noexcept;

    AssetState LoadAnimation (Memory::UniqueString _assetId) noexcept;

    void ProcessUnloading () noexcept;

    void Unload (Memory::UniqueString _assetId) noexcept;

    InsertShortTermQuery insertAssetStateEvent;
    FetchSequenceQuery fetchAssetRemovedEvents;
    FetchValueQuery fetchAssetByTypeNumberAndState;

    InsertLongTermQuery insertAnimation;
    RemoveValueQuery removeAnimationById;

    Container::Vector<Memory::UniqueString> animationRootPaths {Memory::Profiler::AllocationGroup::Top ()};
    const uint64_t maxLoadingTimePerFrameNs;

    Serialization::FieldNameLookupCache animationAssetLookupCache {Sprite2dUvAnimationAssetHeader::Reflect ().mapping};
    Serialization::Yaml::ObjectBundleDeserializer frameBundleDeserializer {
        Sprite2dUvAnimationFrameBundleItem::Reflect ().mapping};
};

Manager::Manager (TaskConstructor &_constructor,
                  const Container::Vector<Memory::UniqueString> &_animationRootPaths,
                  uint64_t _maxLoadingTimePerFrameNs,
                  const StandardLayout::Mapping &_stateUpdateEvent) noexcept
    : insertAssetStateEvent (_constructor.InsertShortTerm (_stateUpdateEvent)),
      fetchAssetRemovedEvents (FETCH_SEQUENCE (AssetRemovedNormalEvent)),
      fetchAssetByTypeNumberAndState (FETCH_VALUE_2F (Asset, typeNumber, state)),

      insertAnimation (INSERT_LONG_TERM (Sprite2dUvAnimation)),
      removeAnimationById (REMOVE_VALUE_1F (Sprite2dUvAnimation, assetId)),
      maxLoadingTimePerFrameNs (_maxLoadingTimePerFrameNs)
{
    animationRootPaths.reserve (_animationRootPaths.size ());
    for (Memory::UniqueString animationRoot : _animationRootPaths)
    {
        animationRootPaths.emplace_back (animationRoot);
    }

    _constructor.DependOn (AssetManagement::Checkpoint::ASSET_LOADING_STARTED);
    _constructor.MakeDependencyOf (AssetManagement::Checkpoint::ASSET_LOADING_FINISHED);
}

void Manager::Execute () noexcept
{
    ProcessLoading ();
    ProcessUnloading ();
}

void Manager::ProcessLoading () noexcept
{
    struct
    {
        StandardLayout::Mapping mapping = Sprite2dUvAnimation::Reflect ().mapping;
        AssetState state = AssetState::LOADING;
    } loadingSprite2dUvAnimationsParameter;

    const std::uint64_t startTime = Emergence::Time::NanosecondsSinceStartup ();
    for (auto assetCursor = fetchAssetByTypeNumberAndState.Execute (&loadingSprite2dUvAnimationsParameter);
         const auto *asset = static_cast<const Asset *> (*assetCursor); ++assetCursor)
    {
        if (Emergence::Time::NanosecondsSinceStartup () - startTime > maxLoadingTimePerFrameNs)
        {
            break;
        }

        // If we're reloading animation, unload old one first.
        Unload (asset->id);

        auto eventCursor = insertAssetStateEvent.Execute ();
        auto *event = static_cast<AssetStateUpdateEventView *> (++eventCursor);
        event->assetId = asset->id;
        event->state = LoadAnimation (asset->id);
        EMERGENCE_ASSERT (event->state != AssetState::LOADING);
    }
}

AssetState Manager::LoadAnimation (Memory::UniqueString _assetId) noexcept
{
    Sprite2dUvAnimationAssetHeader animationHeader;
    bool loaded = false;

    for (Memory::UniqueString root : animationRootPaths)
    {
        std::filesystem::path binaryAnimationPath = EMERGENCE_BUILD_STRING (root, "/", _assetId, ".animation.bin");
        if (std::filesystem::exists (binaryAnimationPath))
        {
            std::ifstream input {binaryAnimationPath, std::ios::binary};
            if (!Serialization::Binary::DeserializeObject (input, &animationHeader,
                                                           Sprite2dUvAnimationAssetHeader::Reflect ().mapping))
            {
                EMERGENCE_LOG (
                    ERROR, "Sprite2dUvAnimationManagement: Unable to load animation from \"",
                    binaryAnimationPath.generic_string<char, std::char_traits<char>, Memory::HeapSTD<char>> (), "\".");
                return AssetState::CORRUPTED;
            }

            loaded = true;
            break;
        }

        std::filesystem::path yamlAnimationPath = EMERGENCE_BUILD_STRING (root, "/", _assetId, ".animation.yaml");
        if (std::filesystem::exists (yamlAnimationPath))
        {
            std::ifstream input {yamlAnimationPath};
            if (!Serialization::Yaml::DeserializeObject (input, &animationHeader, animationAssetLookupCache))
            {
                EMERGENCE_LOG (ERROR, "Sprite2dUvAnimationManagement: Unable to load animation from \"",
                               yamlAnimationPath.generic_string<char, std::char_traits<char>, Memory::HeapSTD<char>> (),
                               "\".");
                return AssetState::CORRUPTED;
            }

            loaded = true;
            break;
        }
    }

    if (!loaded)
    {
        EMERGENCE_LOG (ERROR, "Sprite2dUvAnimationManagement: Unable to find animation \"", _assetId, "\".");
        return AssetState::MISSING;
    }

    auto animationCursor = insertAnimation.Execute ();
    auto *animation = static_cast<Sprite2dUvAnimation *> (++animationCursor);
    animation->assetId = _assetId;
    animation->materialInstanceId = animationHeader.materialInstanceId;
    Sprite2dUvAnimationFrameBundleItem frameItem;

    auto bakeStartTimes = [animation] ()
    {
        for (size_t index = 1u; index < animation->frames.size (); ++index)
        {
            animation->frames[index].startTimeNs =
                animation->frames[index - 1u].startTimeNs + animation->frames[index - 1u].durationNs;
        }

        animation->frames.shrink_to_fit ();
    };

    for (Memory::UniqueString root : animationRootPaths)
    {
        std::filesystem::path binaryFramesPath = EMERGENCE_BUILD_STRING (root, "/", _assetId, ".frames.bin");
        if (std::filesystem::exists (binaryFramesPath))
        {
            std::ifstream input {binaryFramesPath, std::ios::binary};
            // We need to do get-unget in order to force empty file check. Otherwise, it is not guaranteed.
            input.get ();
            input.unget ();

            while (input)
            {
                if (!Serialization::Binary::DeserializeObject (input, &frameItem,
                                                               Sprite2dUvAnimationFrameBundleItem::Reflect ().mapping))
                {
                    EMERGENCE_LOG (
                        ERROR, "Sprite2dUvAnimationManagement: Unable to deserialize frame bundle \"",
                        binaryFramesPath.generic_string<char, std::char_traits<char>, Memory::HeapSTD<char>> (), "\".");
                    return AssetState::CORRUPTED;
                }

                animation->frames.emplace_back () = {frameItem.uv, static_cast<uint64_t> (frameItem.durationS * 1e9f),
                                                     0u};
                // Use peek to test for the end of file or other problems in given stream.
                input.peek ();
            }

            bakeStartTimes ();
            return AssetState::READY;
        }

        std::filesystem::path yamlFramesPath = EMERGENCE_BUILD_STRING (root, "/", _assetId, ".frames.yaml");
        if (std::filesystem::exists (yamlFramesPath))
        {
            std::ifstream input {yamlFramesPath};
            bool successful = frameBundleDeserializer.Begin (input);

            while (successful && frameBundleDeserializer.HasNext ())
            {
                if ((successful = frameBundleDeserializer.Next (&frameItem)))
                {
                    animation->frames.emplace_back () = {frameItem.uv,
                                                         static_cast<uint64_t> (frameItem.durationS * 1e9f), 0u};
                }
            }

            frameBundleDeserializer.End ();
            if (!successful)
            {
                EMERGENCE_LOG (ERROR, "Sprite2dUvAnimationManagement: Unable to deserialize frame bundle \"",
                               yamlFramesPath.generic_string<char, std::char_traits<char>, Memory::HeapSTD<char>> (),
                               "\".");
                return AssetState::CORRUPTED;
            }

            bakeStartTimes ();
            return AssetState::READY;
        }
    }

    EMERGENCE_LOG (ERROR, "Sprite2dUvAnimationManagement: Unable to find animation frames \"", _assetId, "\".");
    return AssetState::MISSING;
}

void Manager::ProcessUnloading () noexcept
{
    for (auto eventCursor = fetchAssetRemovedEvents.Execute ();
         const auto *event = static_cast<const AssetRemovedNormalEvent *> (*eventCursor); ++eventCursor)
    {
        Unload (event->id);
    }
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
                        uint64_t _maxLoadingTimePerFrameNs,
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
        .SetExecutor<Manager> (_animationRootPaths, _maxLoadingTimePerFrameNs, iterator->second);
}
} // namespace Emergence::Celerity::Sprite2dUvAnimationManagement

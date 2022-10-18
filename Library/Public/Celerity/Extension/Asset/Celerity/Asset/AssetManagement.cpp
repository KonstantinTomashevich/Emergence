#include <Celerity/Asset/Asset.hpp>
#include <Celerity/Asset/AssetManagement.hpp>
#include <Celerity/Asset/AssetManagerSingleton.hpp>
#include <Celerity/Asset/Events.hpp>
#include <Celerity/PipelineBuilderMacros.hpp>

#include <Log/Log.hpp>

namespace Emergence::Celerity::AssetManagement
{
class AssetManager final : public TaskExecutorBase<AssetManager>
{
public:
    AssetManager (TaskConstructor &_constructor,
                  const AssetReferenceBindingList &_bindingList,
                  const AssetReferenceBindingEventMap &_bindingEvents) noexcept;

    void Execute () noexcept;

private:
    struct AssetReferenceFieldData
    {
        StandardLayout::Field field;
        StandardLayout::Mapping type;
    };

    struct AssetUserData final
    {
        AssetUserData (TaskConstructor &_constructor,
                       const AssetReferenceBinding &_binding,
                       const AssetReferenceBindingHookEvents &_hooks) noexcept;

        FetchValueQuery fetchUserById;
        FetchSequenceQuery fetchOnAddedEvents;
        FetchSequenceQuery fetchOnChangedEvents;
        FetchSequenceQuery fetchOnRemovedEvents;
        Container::Vector<AssetReferenceFieldData> fields {Memory::Profiler::AllocationGroup::Top ()};
    };

    static void OnAssetRemoved (AssetManagerSingleton *_assetManager, Asset *_asset) noexcept;

    void OnAssetUsageAdded (AssetManagerSingleton *_assetManager,
                            Memory::UniqueString _assetId,
                            const StandardLayout::Mapping &_assetType) noexcept;

    void OnAssetUsageRemoved (AssetManagerSingleton *_assetManager,
                              Memory::UniqueString _assetId,
                              bool _delayAssetRemoval) noexcept;

    Container::Vector<AssetUserData> assetUsers {Memory::Profiler::AllocationGroup::Top ()};
    Container::Vector<Memory::UniqueString> delayedAssetRemovalQueue {Memory::Profiler::AllocationGroup::Top ()};

    ModifySingletonQuery modifyAssetManager;
    InsertLongTermQuery insertAsset;
    ModifyValueQuery modifyAssetById;
};

AssetManager::AssetManager (TaskConstructor &_constructor,
                            const AssetReferenceBindingList &_bindingList,
                            const AssetReferenceBindingEventMap &_bindingEvents) noexcept
    : modifyAssetManager (MODIFY_SINGLETON (AssetManagerSingleton)),
      insertAsset (INSERT_LONG_TERM (Asset)),
      modifyAssetById (MODIFY_VALUE_1F (Asset, id))
{
    _constructor.DependOn (Checkpoint::STARTED);
    _constructor.MakeDependencyOf (Checkpoint::ASSET_LOADING_STARTED);
    _constructor.MakeDependencyOf (Checkpoint::ASSET_LOADING_FINISHED);
    assetUsers.reserve (_bindingList.size ());

    for (const AssetReferenceBinding &binding : _bindingList)
    {
        assert (_bindingEvents.hooks.contains (binding.objectType));
        assetUsers.emplace_back (_constructor, binding, _bindingEvents.hooks.at (binding.objectType));
    }
}

void AssetManager::Execute () noexcept
{
    auto assetManagerCursor = modifyAssetManager.Execute ();
    auto *assetManager = static_cast<AssetManagerSingleton *> (*assetManagerCursor);

    for (AssetUserData &assetUser : assetUsers)
    {
        for (auto eventCursor = assetUser.fetchOnAddedEvents.Execute ();
             const auto *event = static_cast<const AssetUserAddedEventView *> (*eventCursor); ++eventCursor)
        {
            auto assetUserCursor = assetUser.fetchUserById.Execute (&event->assetUserId);
            if (const void *assetUserObject = *assetUserCursor)
            {
                for (const AssetReferenceFieldData &field : assetUser.fields)
                {
                    OnAssetUsageAdded (
                        assetManager,
                        *static_cast<const Memory::UniqueString *> (field.field.GetValue (assetUserObject)),
                        field.type);
                }
            }
        }
    }

#ifndef _NDEBUG
    Container::HashSet<UniqueId> assetUsersChangedThisFrame {assetUsers.get_allocator ()};
#endif

    for (AssetUserData &assetUser : assetUsers)
    {
        for (auto eventCursor = assetUser.fetchOnChangedEvents.Execute ();
             const auto *event = static_cast<const AssetUserChangedEventView *> (*eventCursor); ++eventCursor)
        {
            // Current scenario supports only one change per asset user per frame.
            // Having more than one change per frame indicates that asset usage
            // selection is decentralized, which is not a good pattern in most cases.
            assert (assetUsersChangedThisFrame.emplace (event->assetUserId).second);
            auto assetUserCursor = assetUser.fetchUserById.Execute (&event->assetUserId);

            if (const void *assetUserObject = *assetUserCursor)
            {
                for (std::size_t fieldIndex = 0u; fieldIndex < assetUser.fields.size (); ++fieldIndex)
                {
                    const AssetReferenceFieldData &field = assetUser.fields[fieldIndex];
                    const Memory::UniqueString oldValue = event->unchangedAssets[fieldIndex];
                    const Memory::UniqueString currentValue =
                        *static_cast<const Memory::UniqueString *> (field.field.GetValue (assetUserObject));

                    if (oldValue != currentValue)
                    {
                        OnAssetUsageRemoved (assetManager, oldValue, true);
                        OnAssetUsageAdded (assetManager, currentValue, field.type);
                    }
                }
            }
        }
    }

    for (Memory::UniqueString assetId : delayedAssetRemovalQueue)
    {
        auto cursor = modifyAssetById.Execute (&assetId);
        if (auto *asset = static_cast<Asset *> (*cursor))
        {
            // Usages count might be updated during changes processing, so we need to check it again.
            if (asset->usages == 0u)
            {
                OnAssetRemoved (assetManager, asset);
                ~cursor;
            }
        }
    }

    delayedAssetRemovalQueue.clear ();

    for (AssetUserData &assetUser : assetUsers)
    {
        for (auto eventCursor = assetUser.fetchOnAddedEvents.Execute ();
             const auto *event = static_cast<const AssetUserRemovedEventView *> (*eventCursor); ++eventCursor)
        {
            for (std::size_t fieldIndex = 0u; fieldIndex < assetUser.fields.size (); ++fieldIndex)
            {
                OnAssetUsageRemoved (assetManager, event->assets[fieldIndex], false);
            }
        }
    }

    // Assert that unused asset counting works as expected.
    assert (!assetManager->automaticallyCleanUnusedAssets || assetManager->unusedAssetCount == 0u);
}

AssetManager::AssetUserData::AssetUserData (TaskConstructor &_constructor,
                                            const AssetReferenceBinding &_binding,
                                            const AssetReferenceBindingHookEvents &_hooks) noexcept
    : fetchUserById (_constructor.FetchValue (_binding.objectType, {_binding.resourceUserIdField})),
      fetchOnAddedEvents (_constructor.FetchSequence (_hooks.onObjectAdded)),
      fetchOnChangedEvents (_constructor.FetchSequence (_hooks.onAnyReferenceChanged)),
      fetchOnRemovedEvents (_constructor.FetchSequence (_hooks.onObjectRemoved))
{
    fields.reserve (_binding.references.size ());
    for (const AssetReferenceField &field : _binding.references)
    {
        fields.emplace_back () = {_binding.objectType.GetField (field.field), field.assetType};
    }
}

void AssetManager::OnAssetRemoved (AssetManagerSingleton *_assetManager, Asset *_asset) noexcept
{
    --_assetManager->unusedAssetCount;
    if (_asset->state == AssetState::LOADING)
    {
        --_assetManager->assetsLeftToLoad;
    }
}

void AssetManager::OnAssetUsageAdded (AssetManagerSingleton *_assetManager,
                                      Memory::UniqueString _assetId,
                                      const StandardLayout::Mapping &_assetType) noexcept
{
    if (!*_assetId)
    {
        return;
    }

    // We need a scope because of access management.
    {
        auto cursor = modifyAssetById.Execute (&_assetId);
        if (auto *asset = static_cast<Asset *> (*cursor))
        {
            if (asset->type != _assetType)
            {
                EMERGENCE_LOG (ERROR, "AssetManager: Caught request for asset \"", _assetId, "\" with type \"",
                               _assetType.GetName (), "\", but it already exists with type \"", asset->type.GetName (),
                               "\"!");
            }

            if (++asset->usages == 1u)
            {
                assert (_assetManager->unusedAssetCount > 0u);
                --_assetManager->unusedAssetCount;
            }

            return;
        }
    }

    auto cursor = insertAsset.Execute ();
    auto *asset = static_cast<Asset *> (++cursor);

    asset->id = _assetId;
    asset->type = _assetType;
    asset->usages = 1u;
    asset->state = AssetState::LOADING;
    ++_assetManager->assetsLeftToLoad;
}

void AssetManager::OnAssetUsageRemoved (AssetManagerSingleton *_assetManager,
                                        Memory::UniqueString _assetId,
                                        bool _delayAssetRemoval) noexcept
{
    if (!*_assetId)
    {
        return;
    }

    auto cursor = modifyAssetById.Execute (&_assetId);
    if (auto *asset = static_cast<Asset *> (*cursor))
    {
        assert (asset->usages > 0u);
        if (--asset->usages == 0u)
        {
            ++_assetManager->unusedAssetCount;
            if (_assetManager->automaticallyCleanUnusedAssets)
            {
                if (_delayAssetRemoval)
                {
                    delayedAssetRemovalQueue.emplace_back (_assetId);
                }
                else
                {
                    OnAssetRemoved (_assetManager, asset);
                    ~cursor;
                }
            }
        }
    }
}

class AssetStateUpdater final : public TaskExecutorBase<AssetStateUpdater>
{
public:
    AssetStateUpdater (TaskConstructor &_constructor, const AssetReferenceBindingEventMap &_bindingEvents) noexcept;

    void Execute () noexcept;

private:
    ModifySingletonQuery modifyAssetManager;
    EditValueQuery editAssetById;
    Container::Vector<FetchSequenceQuery> fetchStateUpdateEvents {Memory::Profiler::AllocationGroup::Top ()};
};

AssetStateUpdater::AssetStateUpdater (TaskConstructor &_constructor,
                                      const AssetReferenceBindingEventMap &_bindingEvents) noexcept
    : modifyAssetManager (MODIFY_SINGLETON (AssetManagerSingleton)),
      editAssetById (EDIT_VALUE_1F (Asset, id))
{
    _constructor.DependOn (Checkpoint::ASSET_LOADING_FINISHED);
    _constructor.MakeDependencyOf (Checkpoint::FINISHED);
    fetchStateUpdateEvents.reserve (_bindingEvents.stateUpdate.size ());

    for (const auto &assetTypeEventPair : _bindingEvents.stateUpdate)
    {
        fetchStateUpdateEvents.emplace_back (_constructor.FetchSequence (assetTypeEventPair.second));
    }
}

void AssetStateUpdater::Execute () noexcept
{
    auto assetManagerCursor = modifyAssetManager.Execute ();
    auto *assetManager = static_cast<AssetManagerSingleton *> (*assetManagerCursor);

#ifndef _NDEBUG
    Container::HashSet<Memory::UniqueString> assetsUpdatedThisFrame {fetchStateUpdateEvents.get_allocator ()};
#endif

    for (FetchSequenceQuery &query : fetchStateUpdateEvents)
    {
        for (auto eventCursor = query.Execute ();
             const auto *event = static_cast<const AssetStateUpdateEventView *> (*eventCursor); ++eventCursor)
        {
            // We expect only one update per asset per frame.
            assert (assetsUpdatedThisFrame.emplace (event->assetId).second);
            auto assetCursor = editAssetById.Execute (&event->assetId);

            if (auto *asset = static_cast<Asset *> (*assetCursor))
            {
                if (asset->state != event->state)
                {
                    asset->state = event->state;
                    switch (asset->state)
                    {
                    case AssetState::LOADING:
                        ++assetManager->assetsLeftToLoad;
                        break;

                    case AssetState::MISSING:
                    case AssetState::CORRUPTED:
                    case AssetState::READY:
                        --assetManager->assetsLeftToLoad;
                        break;
                    }
                }
            }
        }
    }
}

void AddToNormalUpdate (PipelineBuilder &_pipelineBuilder,
                        const AssetReferenceBindingList &_bindingList,
                        const AssetReferenceBindingEventMap &_bindingEvents) noexcept
{
    using namespace Memory::Literals;

    _pipelineBuilder.AddCheckpoint (Checkpoint::STARTED);
    _pipelineBuilder.AddCheckpoint (Checkpoint::ASSET_LOADING_STARTED);
    _pipelineBuilder.AddCheckpoint (Checkpoint::ASSET_LOADING_FINISHED);
    _pipelineBuilder.AddCheckpoint (Checkpoint::FINISHED);

    _pipelineBuilder.AddTask ("AssetManager"_us).SetExecutor<AssetManager> (_bindingList, _bindingEvents);
    _pipelineBuilder.AddTask ("AssetStateUpdater"_us).SetExecutor<AssetStateUpdater> (_bindingEvents);
}
} // namespace Emergence::Celerity::AssetManagement

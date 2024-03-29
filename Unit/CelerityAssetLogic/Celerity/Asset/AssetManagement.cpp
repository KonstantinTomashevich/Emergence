#include <Celerity/Asset/Asset.hpp>
#include <Celerity/Asset/AssetManagement.hpp>
#include <Celerity/Asset/AssetManagerSingleton.hpp>
#include <Celerity/Asset/Events.hpp>
#include <Celerity/PipelineBuilderMacros.hpp>

#include <Log/Log.hpp>

namespace Emergence::Celerity::AssetManagement
{
const Memory::UniqueString Checkpoint::STARTED {"AssetManagementStarted"};
const Memory::UniqueString Checkpoint::ASSET_LOADING_STARTED {"AssetManagementAssetLoadingStarted"};
const Memory::UniqueString Checkpoint::ASSET_LOADING_FINISHED {"AssetManagementAssetLoadingFinished"};
const Memory::UniqueString Checkpoint::FINISHED {"AssetManagementFinished"};

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

        FetchSequenceQuery fetchOnAddedNormalEvents;
        FetchSequenceQuery fetchOnChangedNormalEvents;
        FetchSequenceQuery fetchOnRemovedNormalEvents;

        FetchSequenceQuery fetchOnAddedFixedEvents;
        FetchSequenceQuery fetchOnChangedFixedEvents;
        FetchSequenceQuery fetchOnRemovedFixedEvents;

        Container::Vector<AssetReferenceFieldData> fields {Memory::Profiler::AllocationGroup::Top ()};
    };

    void OnAssetUsageAdded (AssetManagerSingleton *_assetManager,
                            Memory::UniqueString _assetId,
                            const StandardLayout::Mapping &_assetType) noexcept;

    void OnAssetUsageRemoved (AssetManagerSingleton *_assetManager, Memory::UniqueString _assetId) noexcept;

    Container::Vector<AssetUserData> assetUsers {Memory::Profiler::AllocationGroup::Top ()};

    ModifySingletonQuery modifyAssetManager;
    InsertLongTermQuery insertAsset;
    ModifyValueQuery modifyAssetById;
    RemoveSignalQuery removeUnusedAssets;
};

AssetManager::AssetManager (TaskConstructor &_constructor,
                            const AssetReferenceBindingList &_bindingList,
                            const AssetReferenceBindingEventMap &_bindingEvents) noexcept
    : TaskExecutorBase (_constructor),

      modifyAssetManager (MODIFY_SINGLETON (AssetManagerSingleton)),
      insertAsset (INSERT_LONG_TERM (Asset)),
      modifyAssetById (MODIFY_VALUE_1F (Asset, id)),
      removeUnusedAssets (REMOVE_SIGNAL (Asset, usages, 0u))
{
    _constructor.DependOn (Checkpoint::STARTED);
    _constructor.MakeDependencyOf (Checkpoint::ASSET_LOADING_STARTED);
    _constructor.MakeDependencyOf (Checkpoint::ASSET_LOADING_FINISHED);
    assetUsers.reserve (_bindingList.size ());

    for (const AssetReferenceBinding &binding : _bindingList)
    {
        EMERGENCE_ASSERT (_bindingEvents.hooks.contains (binding.objectType));
        assetUsers.emplace_back (_constructor, binding, _bindingEvents.hooks.at (binding.objectType));
    }
}

void AssetManager::Execute () noexcept
{
    auto assetManagerCursor = modifyAssetManager.Execute ();
    auto *assetManager = static_cast<AssetManagerSingleton *> (*assetManagerCursor);

    for (AssetUserData &assetUser : assetUsers)
    {
        auto checkEvent = [this, assetManager, &assetUser] (const AssetUserAddedEventView *_event)
        {
            for (std::size_t index = 0u; index < assetUser.fields.size (); ++index)
            {
                OnAssetUsageAdded (assetManager, _event->assetReferences[index], assetUser.fields[index].type);
            }
        };

        for (auto eventCursor = assetUser.fetchOnAddedNormalEvents.Execute ();
             const auto *event = static_cast<const AssetUserAddedEventView *> (*eventCursor); ++eventCursor)
        {
            checkEvent (event);
        }

        for (auto eventCursor = assetUser.fetchOnAddedFixedEvents.Execute ();
             const auto *event = static_cast<const AssetUserAddedEventView *> (*eventCursor); ++eventCursor)
        {
            checkEvent (event);
        }
    }

    for (AssetUserData &assetUser : assetUsers)
    {
        auto checkEvent = [this, assetManager, &assetUser] (const AssetUserChangedEventView *_event)
        {
            for (std::size_t index = 0u; index < assetUser.fields.size (); ++index)
            {
                if (_event->assetReferenceSequence[index] !=
                    _event->assetReferenceSequence[assetUser.fields.size () + index])
                {
                    OnAssetUsageRemoved (assetManager, _event->assetReferenceSequence[index]);
                    OnAssetUsageAdded (assetManager, _event->assetReferenceSequence[assetUser.fields.size () + index],
                                       assetUser.fields[index].type);
                }
            }
        };

        for (auto eventCursor = assetUser.fetchOnChangedNormalEvents.Execute ();
             const auto *event = static_cast<const AssetUserChangedEventView *> (*eventCursor); ++eventCursor)
        {
            checkEvent (event);
        }

        for (auto eventCursor = assetUser.fetchOnChangedFixedEvents.Execute ();
             const auto *event = static_cast<const AssetUserChangedEventView *> (*eventCursor); ++eventCursor)
        {
            checkEvent (event);
        }
    }

    for (AssetUserData &assetUser : assetUsers)
    {
        auto checkEvent = [this, assetManager, &assetUser] (const AssetUserRemovedEventView *_event)
        {
            for (std::size_t index = 0u; index < assetUser.fields.size (); ++index)
            {
                OnAssetUsageRemoved (assetManager, _event->assetReferences[index]);
            }
        };

        for (auto eventCursor = assetUser.fetchOnRemovedNormalEvents.Execute ();
             const auto *event = static_cast<const AssetUserRemovedEventView *> (*eventCursor); ++eventCursor)
        {
            checkEvent (event);
        }

        for (auto eventCursor = assetUser.fetchOnRemovedFixedEvents.Execute ();
             const auto *event = static_cast<const AssetUserRemovedEventView *> (*eventCursor); ++eventCursor)
        {
            checkEvent (event);
        }
    }

    if (assetManager->automaticallyCleanUnusedAssets)
    {
        for (auto cursor = removeUnusedAssets.Execute ();
             const auto *asset = static_cast<const Asset *> (cursor.ReadConst ()); ~cursor)
        {
            --assetManager->unusedAssetCount;
            if (asset->state == AssetState::LOADING)
            {
                --assetManager->assetsLeftToLoad;
            }
        }
    }

    // Assert that unused asset counting works as expected.
    EMERGENCE_ASSERT (!assetManager->automaticallyCleanUnusedAssets || assetManager->unusedAssetCount == 0u);
}

AssetManager::AssetUserData::AssetUserData (TaskConstructor &_constructor,
                                            const AssetReferenceBinding &_binding,
                                            const AssetReferenceBindingHookEvents &_hooks) noexcept
    : fetchOnAddedNormalEvents (_constructor.FetchSequence (_hooks.onObjectAddedNormal)),
      fetchOnChangedNormalEvents (_constructor.FetchSequence (_hooks.onAnyReferenceChangedNormal)),
      fetchOnRemovedNormalEvents (_constructor.FetchSequence (_hooks.onObjectRemovedNormal)),

      fetchOnAddedFixedEvents (_constructor.FetchSequence (_hooks.onObjectAddedFixedToNormal)),
      fetchOnChangedFixedEvents (_constructor.FetchSequence (_hooks.onAnyReferenceChangedFixedToNormal)),
      fetchOnRemovedFixedEvents (_constructor.FetchSequence (_hooks.onObjectRemovedFixedToNormal))
{
    fields.reserve (_binding.references.size ());
    for (const AssetReferenceField &field : _binding.references)
    {
        fields.emplace_back () = {_binding.objectType.GetField (field.field), field.assetType};
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
                EMERGENCE_ASSERT (_assetManager->unusedAssetCount > 0u);
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

void AssetManager::OnAssetUsageRemoved (AssetManagerSingleton *_assetManager, Memory::UniqueString _assetId) noexcept
{
    if (!*_assetId)
    {
        return;
    }

    auto cursor = modifyAssetById.Execute (&_assetId);
    if (auto *asset = static_cast<Asset *> (*cursor))
    {
        EMERGENCE_ASSERT (asset->usages > 0u);
        if (--asset->usages == 0u)
        {
            ++_assetManager->unusedAssetCount;
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
    : TaskExecutorBase (_constructor),

      modifyAssetManager (MODIFY_SINGLETON (AssetManagerSingleton)),
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

#if defined(EMERGENCE_ASSERT_ENABLED)
    Container::HashSet<Memory::UniqueString> assetsUpdatedThisFrame {fetchStateUpdateEvents.get_allocator ()};
#endif

    for (FetchSequenceQuery &query : fetchStateUpdateEvents)
    {
        for (auto eventCursor = query.Execute ();
             const auto *event = static_cast<const AssetStateUpdateEventView *> (*eventCursor); ++eventCursor)
        {
            // We expect only one update per asset per frame.
            EMERGENCE_ASSERT (assetsUpdatedThisFrame.emplace (event->assetId).second);
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

    auto visualGroup = _pipelineBuilder.OpenVisualGroup ("AssetManagement");
    _pipelineBuilder.AddCheckpoint (Checkpoint::STARTED);
    _pipelineBuilder.AddCheckpoint (Checkpoint::ASSET_LOADING_STARTED);
    _pipelineBuilder.AddCheckpoint (Checkpoint::ASSET_LOADING_FINISHED);
    _pipelineBuilder.AddCheckpoint (Checkpoint::FINISHED);

    _pipelineBuilder.AddTask ("AssetManager"_us).SetExecutor<AssetManager> (_bindingList, _bindingEvents);
    _pipelineBuilder.AddTask ("AssetStateUpdater"_us).SetExecutor<AssetStateUpdater> (_bindingEvents);
}
} // namespace Emergence::Celerity::AssetManagement

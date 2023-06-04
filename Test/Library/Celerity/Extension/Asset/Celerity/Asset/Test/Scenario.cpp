#include <Celerity/Asset/AssetManagement.hpp>
#include <Celerity/Asset/AssetManagerSingleton.hpp>
#include <Celerity/Asset/Events.hpp>
#include <Celerity/Asset/Test/Data.hpp>
#include <Celerity/Asset/Test/Scenario.hpp>
#include <Celerity/Event/EventRegistrar.hpp>
#include <Celerity/PipelineBuilder.hpp>
#include <Celerity/PipelineBuilderMacros.hpp>

#include <Testing/Testing.hpp>

namespace Emergence::Celerity::Test
{
using ConfiguratorFrame = Container::Vector<ConfiguratorTask>;
using AssetStateUpdaterFrame = Container::Vector<AssetStateUpdaterTask>;
using VerifierFrame = Container::Vector<VerifierTask>;

class Configurator final : public TaskExecutorBase<Configurator>
{
public:
    Configurator (TaskConstructor &_constructor, Container::Vector<ConfiguratorFrame> _frames) noexcept;

    void Execute () noexcept;

private:
    std::size_t currentFrameIndex = 0u;
    Container::Vector<ConfiguratorFrame> frames;
    ModifySingletonQuery modifyAssetManager;

    InsertLongTermQuery insertSingleAssetUser;
    ModifyValueQuery modifySingleAssetUserById;

    InsertLongTermQuery insertMultipleAssetUser;
    ModifyValueQuery modifyMultipleAssetUserById;
};

Configurator::Configurator (TaskConstructor &_constructor, Container::Vector<ConfiguratorFrame> _frames) noexcept
    : TaskExecutorBase (_constructor),

      frames (std::move (_frames)),
      modifyAssetManager (MODIFY_SINGLETON (AssetManagerSingleton)),

      insertSingleAssetUser (INSERT_LONG_TERM (SingleAssetUser)),
      modifySingleAssetUserById (MODIFY_VALUE_1F (SingleAssetUser, assetUserId)),

      insertMultipleAssetUser (INSERT_LONG_TERM (MultipleAssetUser)),
      modifyMultipleAssetUserById (MODIFY_VALUE_1F (MultipleAssetUser, assetUserId))
{
    _constructor.MakeDependencyOf (AssetManagement::Checkpoint::STARTED);
}

void Configurator::Execute () noexcept
{
    using namespace ConfiguratorTasks;
    REQUIRE (currentFrameIndex < frames.size ());
    const ConfiguratorFrame &frame = frames[currentFrameIndex];

    for (const ConfiguratorTask &task : frame)
    {
        std::visit (
            [this] (const auto &_task)
            {
                using Type = std::decay_t<decltype (_task)>;
                if constexpr (std::is_same_v<Type, SetAutomaticallyCleanUnusedAssets>)
                {
                    LOG ((_task.automaticallyCleanUnusedAssets ? "Enabling" : "Disabling"),
                         " automatic cleanup for unused assets.");

                    auto cursor = modifyAssetManager.Execute ();
                    auto *manager = static_cast<AssetManagerSingleton *> (*cursor);
                    manager->automaticallyCleanUnusedAssets = _task.automaticallyCleanUnusedAssets;
                }
                else if constexpr (std::is_same_v<Type, AddSingleAssetUser>)
                {
                    LOG ("Adding SingleAssetUser with id ", _task.assetUserId, " and asset reference \"", _task.assetId,
                         "\".");

                    auto cursor = insertSingleAssetUser.Execute ();
                    auto *assetUser = static_cast<SingleAssetUser *> (++cursor);
                    assetUser->assetUserId = _task.assetUserId;
                    assetUser->assetId = _task.assetId;
                }
                else if constexpr (std::is_same_v<Type, ChangeSingleAssetUser>)
                {
                    LOG ("Setting asset reference of SingleAssetUser with id ", _task.assetUserId, " to \"",
                         _task.assetId, "\".");

                    auto cursor = modifySingleAssetUserById.Execute (&_task.assetUserId);
                    if (auto *assetUser = static_cast<SingleAssetUser *> (*cursor))
                    {
                        assetUser->assetId = _task.assetId;
                    }
                    else
                    {
                        CHECK_WITH_MESSAGE (false, "Unable to find SingleAssetUser with id ", _task.assetUserId, "!");
                    }
                }
                else if constexpr (std::is_same_v<Type, RemoveSingleAssetUser>)
                {
                    LOG ("Removing SingleAssetUser with id ", _task.assetUserId, ".");
                    auto cursor = modifySingleAssetUserById.Execute (&_task.assetUserId);
                    if (*cursor)
                    {
                        ~cursor;
                    }
                    else
                    {
                        CHECK_WITH_MESSAGE (false, "Unable to find SingleAssetUser with id ", _task.assetUserId, "!");
                    }
                }
                else if constexpr (std::is_same_v<Type, AddMultipleAssetUser>)
                {
                    LOG ("Adding MultipleAssetUser with id ", _task.assetUserId, " and asset references \"",
                         _task.firstAssetId, "\", \"", _task.secondAssetId, "\" and \"", _task.thirdAssetId, "\".");

                    auto cursor = insertMultipleAssetUser.Execute ();
                    auto *assetUser = static_cast<MultipleAssetUser *> (++cursor);
                    assetUser->assetUserId = _task.assetUserId;
                    assetUser->firstAssetId = _task.firstAssetId;
                    assetUser->secondAssetId = _task.secondAssetId;
                    assetUser->thirdAssetId = _task.thirdAssetId;
                }
                else if constexpr (std::is_same_v<Type, ChangeMultipleAssetUser>)
                {
                    LOG ("Setting asset references of MultipleAssetUser with id ", _task.assetUserId, " to \"",
                         _task.firstAssetId, "\", \"", _task.secondAssetId, "\" and \"", _task.thirdAssetId, "\".");

                    auto cursor = modifyMultipleAssetUserById.Execute (&_task.assetUserId);
                    if (auto *assetUser = static_cast<MultipleAssetUser *> (*cursor))
                    {
                        assetUser->firstAssetId = _task.firstAssetId;
                        assetUser->secondAssetId = _task.secondAssetId;
                        assetUser->thirdAssetId = _task.thirdAssetId;
                    }
                    else
                    {
                        CHECK_WITH_MESSAGE (false, "Unable to find MultipleAssetUser with id ", _task.assetUserId, "!");
                    }
                }
                else if constexpr (std::is_same_v<Type, RemoveMultipleAssetUser>)
                {
                    LOG ("Removing MultipleAssetUser with id ", _task.assetUserId, ".");
                    auto cursor = modifyMultipleAssetUserById.Execute (&_task.assetUserId);
                    if (*cursor)
                    {
                        ~cursor;
                    }
                    else
                    {
                        CHECK_WITH_MESSAGE (false, "Unable to find MultipleAssetUser with id ", _task.assetUserId, "!");
                    }
                }
            },
            task);
    }

    ++currentFrameIndex;
}

class AssetStateUpdater final : public TaskExecutorBase<AssetStateUpdater>
{
public:
    AssetStateUpdater (TaskConstructor &_constructor,
                       Container::Vector<AssetStateUpdaterFrame> _frames,
                       const AssetReferenceBindingEventMap &_eventMap) noexcept;

    void Execute () noexcept;

private:
    std::size_t currentFrameIndex = 0u;
    Container::Vector<AssetStateUpdaterFrame> frames;
    InsertShortTermQuery insertFirstAssetStateUpdate;
    InsertShortTermQuery insertSecondAssetStateUpdate;
};

AssetStateUpdater::AssetStateUpdater (TaskConstructor &_constructor,
                                      Container::Vector<AssetStateUpdaterFrame> _frames,
                                      const AssetReferenceBindingEventMap &_eventMap) noexcept
    : TaskExecutorBase (_constructor),

      frames (std::move (_frames)),
      insertFirstAssetStateUpdate (
          _constructor.InsertShortTerm (_eventMap.stateUpdate.at (FirstAssetType::Reflect ().mapping))),
      insertSecondAssetStateUpdate (
          _constructor.InsertShortTerm (_eventMap.stateUpdate.at (SecondAssetType::Reflect ().mapping)))
{
    _constructor.DependOn (AssetManagement::Checkpoint::ASSET_LOADING_STARTED);
    _constructor.MakeDependencyOf (AssetManagement::Checkpoint::ASSET_LOADING_FINISHED);
}

void AssetStateUpdater::Execute () noexcept
{
    using namespace AssetStateUpdaterTasks;
    REQUIRE (currentFrameIndex < frames.size ());
    const AssetStateUpdaterFrame &frame = frames[currentFrameIndex];

    for (const AssetStateUpdaterTask &task : frame)
    {
        std::visit (
            [this] (const auto &_task)
            {
                using Type = std::decay_t<decltype (_task)>;
                LOG ("Updating asset \"", _task.assetId, "\" state to ",
                     static_cast<std::underlying_type_t<AssetState>> (_task.state), ".");

                if constexpr (std::is_same_v<Type, UpdateFirstAssetType>)
                {
                    auto cursor = insertFirstAssetStateUpdate.Execute ();
                    auto *event = static_cast<AssetStateUpdateEventView *> (++cursor);
                    event->assetId = _task.assetId;
                    event->state = _task.state;
                }
                else if constexpr (std::is_same_v<Type, UpdateSecondAssetType>)
                {
                    auto cursor = insertSecondAssetStateUpdate.Execute ();
                    auto *event = static_cast<AssetStateUpdateEventView *> (++cursor);
                    event->assetId = _task.assetId;
                    event->state = _task.state;
                }
            },
            task);
    }

    ++currentFrameIndex;
}

class Verifier final : public TaskExecutorBase<Verifier>
{
public:
    Verifier (TaskConstructor &_constructor, Container::Vector<VerifierFrame> _frames) noexcept;

    void Execute () noexcept;

private:
    std::size_t currentFrameIndex = 0u;
    Container::Vector<VerifierFrame> frames;
    FetchSingletonQuery fetchAssetManager;
    FetchValueQuery fetchAssetById;
};

Verifier::Verifier (TaskConstructor &_constructor, Container::Vector<Container::Vector<VerifierTask>> _frames) noexcept
    : TaskExecutorBase (_constructor),

      frames (std::move (_frames)),
      fetchAssetManager (FETCH_SINGLETON (AssetManagerSingleton)),
      fetchAssetById (FETCH_VALUE_1F (Asset, id))
{
    _constructor.DependOn (AssetManagement::Checkpoint::FINISHED);
}

void Verifier::Execute () noexcept
{
    using namespace VerifierTasks;
    REQUIRE (currentFrameIndex < frames.size ());
    const VerifierFrame &frame = frames[currentFrameIndex];

    for (const VerifierTask &task : frame)
    {
        std::visit (
            [this] (const auto &_task)
            {
                using Type = std::decay_t<decltype (_task)>;
                if constexpr (std::is_same_v<Type, CheckAssetsLeftToLoad>)
                {
                    LOG ("Checking that there is ", _task.assetsLeftToLoad, " assets left to load.");
                    auto cursor = fetchAssetManager.Execute ();
                    const auto *manager = static_cast<const AssetManagerSingleton *> (*cursor);
                    CHECK_EQUAL (manager->assetsLeftToLoad, _task.assetsLeftToLoad);
                }
                else if constexpr (std::is_same_v<Type, CheckUnusedAssetCount>)
                {
                    LOG ("Checking that there is ", _task.unusedAssetCount, " unused assets.");
                    auto cursor = fetchAssetManager.Execute ();
                    const auto *manager = static_cast<const AssetManagerSingleton *> (*cursor);
                    CHECK_EQUAL (manager->unusedAssetCount, _task.unusedAssetCount);
                }
                else if constexpr (std::is_same_v<Type, CheckAssetNotExists>)
                {
                    LOG ("Checking that asset with id \"", _task.assetId, "\" not exists.");
                    auto cursor = fetchAssetById.Execute (&_task.assetId);
                    CHECK (!*cursor);
                }
                else if constexpr (std::is_same_v<Type, CheckAssetType>)
                {
                    LOG ("Checking that asset with id \"", _task.assetId, "\" has type \"", _task.type.GetName (),
                         "\".");
                    auto cursor = fetchAssetById.Execute (&_task.assetId);

                    if (const auto *asset = static_cast<const Asset *> (*cursor))
                    {
                        CHECK_EQUAL (asset->type, _task.type);
                    }
                    else
                    {
                        CHECK_WITH_MESSAGE (false, "Unable to find asset with id \"", _task.assetId, "\".");
                    }
                }
                else if constexpr (std::is_same_v<Type, CheckAssetUsages>)
                {
                    LOG ("Checking that asset with id \"", _task.assetId, "\" has \"", _task.usages, " usages.");
                    auto cursor = fetchAssetById.Execute (&_task.assetId);

                    if (const auto *asset = static_cast<const Asset *> (*cursor))
                    {
                        CHECK_EQUAL (asset->usages, _task.usages);
                    }
                    else
                    {
                        CHECK_WITH_MESSAGE (false, "Unable to find asset with id \"", _task.assetId, "\".");
                    }
                }
                else if constexpr (std::is_same_v<Type, CheckAssetState>)
                {
                    LOG ("Checking that asset with id \"", _task.assetId, "\" has state \"",
                         static_cast<std::underlying_type_t<AssetState>> (_task.state), "\".");
                    auto cursor = fetchAssetById.Execute (&_task.assetId);

                    if (const auto *asset = static_cast<const Asset *> (*cursor))
                    {
                        CHECK_EQUAL (asset->state, _task.state);
                    }
                    else
                    {
                        CHECK_WITH_MESSAGE (false, "Unable to find asset with id \"", _task.assetId, "\".");
                    }
                }
            },
            task);
    }

    ++currentFrameIndex;
}

void ExecuteScenario (const Container::Vector<FrameTasks> &_tasks) noexcept
{
    using namespace Memory::Literals;

    const std::size_t frames = _tasks.size ();
    Container::Vector<ConfiguratorFrame> configuratorFrames;
    Container::Vector<AssetStateUpdaterFrame> assetStateUpdaterFrames;
    Container::Vector<VerifierFrame> verifierFrames;

    for (const FrameTasks &frame : _tasks)
    {
        configuratorFrames.emplace_back (frame.configuratorTasks);
        assetStateUpdaterFrames.emplace_back (frame.assetStateUpdaterTasks);
        verifierFrames.emplace_back (frame.verifierTasks);
    }

    World world {"TestWorld"_us, WorldConfiguration {}};
    const AssetReferenceBindingList assetReferenceBindingList = GetAssetReferenceBinding ();
    AssetReferenceBindingEventMap eventMap;

    {
        EventRegistrar registrar {&world};
        eventMap = RegisterAssetEvents (registrar, assetReferenceBindingList);
    }

    PipelineBuilder builder {world.GetRootView ()};
    builder.Begin ("NormalUpdate"_us, PipelineType::NORMAL);
    builder.AddTask ("Configurator"_us).SetExecutor<Configurator> (std::move (configuratorFrames));
    builder.AddTask ("AssetStateUpdateRequester"_us)
        .SetExecutor<AssetStateUpdater> (std::move (assetStateUpdaterFrames), eventMap);
    builder.AddTask ("Verifier"_us).SetExecutor<Verifier> (std::move (verifierFrames));
    AssetManagement::AddToNormalUpdate (builder, assetReferenceBindingList, eventMap);
    REQUIRE (builder.End ());

    for (std::size_t index = 0u; index < frames; ++index)
    {
        WorldTestingUtility::RunNormalUpdateOnce (world, 1u);
    }
}
} // namespace Emergence::Celerity::Test

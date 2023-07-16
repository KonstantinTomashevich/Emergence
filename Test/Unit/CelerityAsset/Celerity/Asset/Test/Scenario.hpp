#pragma once

#include <Celerity/Asset/Asset.hpp>
#include <Celerity/Standard/UniqueId.hpp>

#include <Container/Variant.hpp>
#include <Container/Vector.hpp>

#include <Memory/Profiler/Test/DefaultAllocationGroupStub.hpp>

namespace Emergence::Celerity::Test
{
namespace ConfiguratorTasks
{
struct SetAutomaticallyCleanUnusedAssets final
{
    bool automaticallyCleanUnusedAssets = false;
};

struct AddSingleAssetUser final
{
    UniqueId assetUserId = INVALID_UNIQUE_ID;
    Memory::UniqueString assetId;
};

struct ChangeSingleAssetUser final
{
    UniqueId assetUserId = INVALID_UNIQUE_ID;
    Memory::UniqueString assetId;
};

struct RemoveSingleAssetUser final
{
    UniqueId assetUserId = INVALID_UNIQUE_ID;
};

struct AddMultipleAssetUser final
{
    UniqueId assetUserId = INVALID_UNIQUE_ID;
    Memory::UniqueString firstAssetId;
    Memory::UniqueString secondAssetId;
    Memory::UniqueString thirdAssetId;
};

struct ChangeMultipleAssetUser final
{
    UniqueId assetUserId = INVALID_UNIQUE_ID;
    Memory::UniqueString firstAssetId;
    Memory::UniqueString secondAssetId;
    Memory::UniqueString thirdAssetId;
};

struct RemoveMultipleAssetUser final
{
    UniqueId assetUserId = INVALID_UNIQUE_ID;
};
} // namespace ConfiguratorTasks

using ConfiguratorTask = Container::Variant<ConfiguratorTasks::SetAutomaticallyCleanUnusedAssets,
                                            ConfiguratorTasks::AddSingleAssetUser,
                                            ConfiguratorTasks::ChangeSingleAssetUser,
                                            ConfiguratorTasks::RemoveSingleAssetUser,
                                            ConfiguratorTasks::AddMultipleAssetUser,
                                            ConfiguratorTasks::ChangeMultipleAssetUser,
                                            ConfiguratorTasks::RemoveMultipleAssetUser>;

namespace AssetStateUpdaterTasks
{
struct UpdateFirstAssetType final
{
    Memory::UniqueString assetId;
    AssetState state = AssetState::LOADING;
};

struct UpdateSecondAssetType final
{
    Memory::UniqueString assetId;
    AssetState state = AssetState::LOADING;
};
} // namespace AssetStateUpdaterTasks

using AssetStateUpdaterTask =
    Container::Variant<AssetStateUpdaterTasks::UpdateFirstAssetType, AssetStateUpdaterTasks::UpdateSecondAssetType>;

namespace VerifierTasks
{
struct CheckAssetsLeftToLoad final
{
    std::uint32_t assetsLeftToLoad = 0u;
};

struct CheckUnusedAssetCount final
{
    std::uint32_t unusedAssetCount = 0u;
};

struct CheckAssetNotExists final
{
    Memory::UniqueString assetId;
};

struct CheckAssetType final
{
    Memory::UniqueString assetId;
    StandardLayout::Mapping type;
};

struct CheckAssetUsages final
{
    Memory::UniqueString assetId;
    std::uint32_t usages = 0u;
};

struct CheckAssetState final
{
    Memory::UniqueString assetId;
    AssetState state = AssetState::LOADING;
};
} // namespace VerifierTasks

using VerifierTask = Container::Variant<VerifierTasks::CheckAssetsLeftToLoad,
                                        VerifierTasks::CheckUnusedAssetCount,
                                        VerifierTasks::CheckAssetNotExists,
                                        VerifierTasks::CheckAssetType,
                                        VerifierTasks::CheckAssetUsages,
                                        VerifierTasks::CheckAssetState>;

struct FrameTasks
{
    Container::Vector<ConfiguratorTask> configuratorTasks;
    Container::Vector<AssetStateUpdaterTask> assetStateUpdaterTasks;
    Container::Vector<VerifierTask> verifierTasks;
};

void ExecuteScenario (const Container::Vector<FrameTasks> &_tasks) noexcept;
} // namespace Emergence::Celerity::Test

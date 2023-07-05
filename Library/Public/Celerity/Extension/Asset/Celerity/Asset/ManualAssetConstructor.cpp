#include <Celerity/Asset/Asset.hpp>
#include <Celerity/Asset/AssetManagerSingleton.hpp>
#include <Celerity/Asset/ManualAssetConstructor.hpp>
#include <Celerity/PipelineBuilderMacros.hpp>

namespace Emergence::Celerity
{
ManualAssetConstructor::ManualAssetConstructor (TaskConstructor &_constructor) noexcept
    : modifyAssetManager (MODIFY_SINGLETON (AssetManagerSingleton)),
      insertAsset (INSERT_LONG_TERM (Asset))
{
}

void ManualAssetConstructor::ConstructManualAsset (Memory::UniqueString _assetId,
                                                   const StandardLayout::Mapping &_assetType) noexcept
{
    auto managerCursor = modifyAssetManager.Execute ();
    auto *manager = static_cast<AssetManagerSingleton *> (*managerCursor);
    ++manager->unusedAssetCount;

    auto assetCursor = insertAsset.Execute ();
    auto *asset = static_cast<Asset *> (++assetCursor);
    asset->id = _assetId;
    asset->type = _assetType;
    asset->state = AssetState::READY;
    asset->usages = 0u;
}
} // namespace Emergence::Celerity

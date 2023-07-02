#include <Celerity/Asset/Asset.hpp>
#include <Celerity/Asset/ManualAssetConstructor.hpp>
#include <Celerity/PipelineBuilderMacros.hpp>

namespace Emergence::Celerity
{
ManualAssetConstructor::ManualAssetConstructor (TaskConstructor &_constructor) noexcept
    : insertAsset (INSERT_LONG_TERM (Asset))
{
}

void ManualAssetConstructor::ConstructManualAsset (Memory::UniqueString _assetId,
                                                   const StandardLayout::Mapping &_assetType) noexcept
{
    auto cursor = insertAsset.Execute ();
    auto *asset = static_cast<Asset *> (++cursor);
    asset->id = _assetId;
    asset->type = _assetType;
    asset->state = AssetState::READY;
    asset->usages = 0u;
}
} // namespace Emergence::Celerity

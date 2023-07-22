#pragma once

#include <CelerityAssetModelApi.hpp>

#include <StandardLayout/Mapping.hpp>

namespace Emergence::Celerity
{
/// \brief Contains global configuration and information about asset management.
struct CelerityAssetModelApi AssetManagerSingleton final
{
    /// \brief Total count of assets that are currently in AssetState::LOADING.
    std::uint32_t assetsLeftToLoad = 0u;

    /// \brief Total count of assets that have no references.
    std::uint32_t unusedAssetCount = 0u;

    /// \brief Indicates whether assets with no references should be deleted automatically.
    bool automaticallyCleanUnusedAssets = true;

    struct CelerityAssetModelApi Reflection final
    {
        StandardLayout::FieldId assetsLeftToLoad;
        StandardLayout::FieldId unusedAssetCount;
        StandardLayout::FieldId automaticallyCleanUnusedAssets;
        StandardLayout::Mapping mapping;
    };

    static const Reflection &Reflect () noexcept;
};
} // namespace Emergence::Celerity

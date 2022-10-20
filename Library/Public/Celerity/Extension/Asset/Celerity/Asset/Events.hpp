#pragma once

#include <Celerity/Asset/Asset.hpp>
#include <Celerity/Asset/AssetReferenceBinding.hpp>
#include <Celerity/Event/EventRegistrar.hpp>
#include <Celerity/Standard/UniqueId.hpp>

namespace Emergence::Celerity
{
/// \brief Any autogenerated asset user addition event can be casted to this type.
struct AssetUserAddedEventView
{
    UniqueId assetUserId;
};

/// \brief Any autogenerated asset user change event can be casted to this type.
struct AssetUserChangedEventView
{
    UniqueId assetUserId;
    Memory::UniqueString unchangedAssets[0u];
};

/// \brief Any autogenerated asset user removal event can be casted to this type.
struct AssetUserRemovedEventView
{
    Memory::UniqueString assets[0u];
};

/// \brief Any autogenerated asset state update request event can be casted to this type.
struct AssetStateUpdateEventView
{
    Memory::UniqueString assetId;
    AssetState state;
};

/// \brief Generates and registers events for all asset users and asset types.
AssetReferenceBindingEventMap RegisterAssetEvents (EventRegistrar &_registrar,
                                                   const AssetReferenceBindingList &_bindings) noexcept;
} // namespace Emergence::Celerity

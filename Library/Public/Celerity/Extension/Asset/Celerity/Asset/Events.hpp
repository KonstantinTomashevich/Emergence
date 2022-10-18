#pragma once

#include <Celerity/Asset/Asset.hpp>
#include <Celerity/Asset/AssetReferenceBinding.hpp>
#include <Celerity/Event/EventRegistrar.hpp>
#include <Celerity/Standard/UniqueId.hpp>

namespace Emergence::Celerity
{
struct AssetUserAddedEventView
{
    UniqueId assetUserId;
};

struct AssetUserChangedEventView
{
    UniqueId assetUserId;
    Memory::UniqueString unchangedAssets[0u];
};

struct AssetUserRemovedEventView
{
    Memory::UniqueString assets[0u];
};

struct AssetStateUpdateEventView
{
    Memory::UniqueString assetId;
    AssetState state;
};

AssetReferenceBindingEventMap RegisterAssetEvents (EventRegistrar &_registrar,
                                                   const AssetReferenceBindingList &_bindings) noexcept;
} // namespace Emergence::Celerity

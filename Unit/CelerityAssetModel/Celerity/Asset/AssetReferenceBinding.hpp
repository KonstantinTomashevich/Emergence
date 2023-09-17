#pragma once

#include <CelerityAssetModelApi.hpp>

#include <Container/HashMap.hpp>
#include <Container/Vector.hpp>

#include <StandardLayout/Mapping.hpp>

namespace Emergence::Celerity
{
/// \return Allocation group for temporary asset reference binding data.
Memory::Profiler::AllocationGroup CelerityAssetModelApi GetAssetBindingAllocationGroup () noexcept;

/// \brief Describes field that references an asset.
struct CelerityAssetModelApi AssetReferenceField final
{
    /// \brief Id of a field with reference.
    /// \invariant Must be FieldArchetype::UNIQUE_STRING.
    StandardLayout::FieldId field;

    /// \brief Type of referenced asset.
    StandardLayout::Mapping assetType;
};

/// \brief Describes an asset user: object that references one or more assets.
struct CelerityAssetModelApi AssetReferenceBinding final
{
    /// \brief Type of an object that contains asset references.
    StandardLayout::Mapping objectType;

    /// \brief Lists fields that reference assets.
    Container::Vector<AssetReferenceField> references {GetAssetBindingAllocationGroup ()};
};

/// \brief Lists all types that reference assets.
using AssetReferenceBindingList = Container::Vector<AssetReferenceBinding>;

/// \brief Stores automatically generated event types for single asset user type.
struct CelerityAssetModelApi AssetReferenceBindingHookEvents final
{
    /// \brief Asset user addition in normal pipeline event that is required for asset usage tracking.
    StandardLayout::Mapping onObjectAddedNormal;

    /// \brief Asset user change in normal pipeline event that is required for asset usage tracking.
    StandardLayout::Mapping onAnyReferenceChangedNormal;

    /// \brief Asset user removal in normal pipeline event that is required for asset usage tracking.
    StandardLayout::Mapping onObjectRemovedNormal;

    /// \brief Asset user addition in fixed pipeline event that is required for asset usage tracking.
    /// \details In ideal world all asset users operate only in normal pipeline, but in some cases
    ///          like debug draw it is convenient to work with them from fixed pipeline too.
    StandardLayout::Mapping onObjectAddedFixedToNormal;

    /// \brief Asset user change in fixed pipeline event that is required for asset usage tracking.
    /// \details In ideal world all asset users operate only in normal pipeline, but in some cases
    ///          like debug draw it is convenient to work with them from fixed pipeline too.
    StandardLayout::Mapping onAnyReferenceChangedFixedToNormal;

    /// \brief Asset user removal in fixed pipeline event that is required for asset usage tracking.
    /// \details In ideal world all asset users operate only in normal pipeline, but in some cases
    ///          like debug draw it is convenient to work with them from fixed pipeline too.
    StandardLayout::Mapping onObjectRemovedFixedToNormal;
};

/// \brief Stores all automatically generated event types.
struct CelerityAssetModelApi AssetReferenceBindingEventMap final
{
    /// \brief Stores all events for asset users.
    Container::HashMap<StandardLayout::Mapping, AssetReferenceBindingHookEvents> hooks {
        GetAssetBindingAllocationGroup ()};

    /// \brief Stores all events for asset state update requests.
    Container::HashMap<StandardLayout::Mapping, StandardLayout::Mapping> stateUpdate {
        GetAssetBindingAllocationGroup ()};
};
} // namespace Emergence::Celerity

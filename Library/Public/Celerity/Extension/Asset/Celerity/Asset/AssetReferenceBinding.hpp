#pragma once

#include <Container/HashMap.hpp>
#include <Container/Vector.hpp>

#include <StandardLayout/Mapping.hpp>

namespace Emergence::Celerity
{
/// \return Allocation group for temporary asset reference binding data.
Memory::Profiler::AllocationGroup GetAssetBindingAllocationGroup () noexcept;

/// \brief Describes field that references an asset.
struct AssetReferenceField final
{
    /// \brief Id of a field with reference.
    /// \invariant Must be FieldArchetype::UNIQUE_STRING.
    StandardLayout::FieldId field;

    /// \brief Type of referenced asset.
    StandardLayout::Mapping assetType;
};

/// \brief Describes an asset user: object that references one or more assets.
struct AssetReferenceBinding final
{
    /// \brief Type of an object that contains asset references.
    StandardLayout::Mapping objectType;

    /// \brief Lists fields that reference assets.
    Container::Vector<AssetReferenceField> references {GetAssetBindingAllocationGroup ()};
};

/// \brief Lists all types that reference assets.
using AssetReferenceBindingList = Container::Vector<AssetReferenceBinding>;

/// \brief Stores automatically generated event types for single asset user type.
struct AssetReferenceBindingHookEvents final
{
    /// \brief Asset user addition event that is required for asset usage tracking.
    StandardLayout::Mapping onObjectAdded;

    /// \brief Asset user change event that is required for asset usage tracking.
    StandardLayout::Mapping onAnyReferenceChanged;

    /// \brief Asset user removal event that is required for asset usage tracking.
    StandardLayout::Mapping onObjectRemoved;
};

/// \brief Stores all automatically generated event types.
struct AssetReferenceBindingEventMap final
{
    /// \brief Stores all events for asset users.
    Container::HashMap<StandardLayout::Mapping, AssetReferenceBindingHookEvents> hooks {
        GetAssetBindingAllocationGroup ()};

    /// \brief Stores all events for asset state update requests.
    Container::HashMap<StandardLayout::Mapping, StandardLayout::Mapping> stateUpdate {
        GetAssetBindingAllocationGroup ()};
};
} // namespace Emergence::Celerity

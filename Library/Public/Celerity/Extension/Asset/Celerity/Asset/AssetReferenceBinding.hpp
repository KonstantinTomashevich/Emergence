#pragma once

#include <Container/HashMap.hpp>
#include <Container/Vector.hpp>

#include <StandardLayout/Mapping.hpp>

namespace Emergence::Celerity
{
Memory::Profiler::AllocationGroup GetAssetBindingAllocationGroup () noexcept;

struct AssetReferenceField final
{
    StandardLayout::FieldId field;
    StandardLayout::Mapping assetType;
};

struct AssetReferenceBinding final
{
    StandardLayout::Mapping objectType;
    StandardLayout::FieldId resourceUserIdField;
    Container::Vector<AssetReferenceField> references {GetAssetBindingAllocationGroup ()};
};

using AssetReferenceBindingList = Container::Vector<AssetReferenceBinding>;

struct AssetReferenceBindingHookEvents final
{
    StandardLayout::Mapping onObjectAdded;
    StandardLayout::Mapping onAnyReferenceChanged;
    StandardLayout::Mapping onObjectRemoved;
};

struct AssetReferenceBindingEventMap final
{
    Container::HashMap<StandardLayout::Mapping, AssetReferenceBindingHookEvents> hooks {
        GetAssetBindingAllocationGroup ()};
    Container::HashMap<StandardLayout::Mapping, StandardLayout::Mapping> stateUpdate {
        GetAssetBindingAllocationGroup ()};
};
} // namespace Emergence::Celerity

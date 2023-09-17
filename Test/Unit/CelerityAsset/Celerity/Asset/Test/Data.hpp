#pragma once

#include <Celerity/Asset/AssetReferenceBinding.hpp>
#include <Celerity/Standard/UniqueId.hpp>

namespace Emergence::Celerity::Test
{
struct FirstAssetType
{
    struct Reflection
    {
        StandardLayout::Mapping mapping;
    };

    static const Reflection &Reflect () noexcept;
};

struct SecondAssetType
{
    struct Reflection
    {
        StandardLayout::Mapping mapping;
    };

    static const Reflection &Reflect () noexcept;
};

struct SingleAssetUser
{
    UniqueId assetUserId = INVALID_UNIQUE_ID;
    Memory::UniqueString assetId;

    struct Reflection
    {
        StandardLayout::FieldId assetUserId;
        StandardLayout::FieldId assetId;
        StandardLayout::Mapping mapping;
    };

    static const Reflection &Reflect () noexcept;
};

struct MultipleAssetUser
{
    UniqueId assetUserId = INVALID_UNIQUE_ID;
    Memory::UniqueString firstAssetId;
    Memory::UniqueString secondAssetId;
    Memory::UniqueString thirdAssetId;

    struct Reflection
    {
        StandardLayout::FieldId assetUserId;
        StandardLayout::FieldId firstAssetId;
        StandardLayout::FieldId secondAssetId;
        StandardLayout::FieldId thirdAssetId;
        StandardLayout::Mapping mapping;
    };

    static const Reflection &Reflect () noexcept;
};

AssetReferenceBindingList GetAssetReferenceBinding () noexcept;
} // namespace Emergence::Celerity::Test

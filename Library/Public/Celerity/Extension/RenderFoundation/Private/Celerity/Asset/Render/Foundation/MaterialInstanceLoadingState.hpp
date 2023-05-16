#pragma once

#include <Celerity/Asset/Asset.hpp>
#include <Celerity/Asset/Render/Foundation/MaterialInstance.hpp>

#include <StandardLayout/Mapping.hpp>

namespace Emergence::Celerity
{
struct MaterialInstanceLoadingState final
{
    Memory::UniqueString assetId;

    Memory::UniqueString parentId;

    std::atomic<AssetState> state {AssetState::LOADING};

    bool valid = true;

    MaterialInstanceAsset asset;

    struct Reflection final
    {
        StandardLayout::FieldId assetId;
        StandardLayout::FieldId parentId;
        StandardLayout::FieldId valid;
        StandardLayout::FieldId asset;
        StandardLayout::Mapping mapping;
    };

    static const Reflection &Reflect () noexcept;
};
} // namespace Emergence::Celerity

#pragma once

#include <Celerity/Asset/Asset.hpp>
#include <Celerity/Asset/Render/Foundation/Material.hpp>

#include <StandardLayout/Mapping.hpp>

namespace Emergence::Celerity
{
struct MaterialLoadingState final
{
    Memory::UniqueString assetId;

    std::atomic<AssetState> state {AssetState::LOADING};

    bool valid = true;

    MaterialAsset asset;

    struct Reflection final
    {
        StandardLayout::FieldId assetId;
        StandardLayout::FieldId valid;
        StandardLayout::FieldId asset;
        StandardLayout::Mapping mapping;
    };

    static const Reflection &Reflect () noexcept;
};
} // namespace Emergence::Celerity

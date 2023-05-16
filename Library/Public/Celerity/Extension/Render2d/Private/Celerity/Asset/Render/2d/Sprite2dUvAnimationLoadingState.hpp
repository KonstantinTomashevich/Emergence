#pragma once

#include <Celerity/Asset/Asset.hpp>
#include <Celerity/Asset/Render/2d/Sprite2dUvAnimation.hpp>

#include <StandardLayout/Mapping.hpp>

namespace Emergence::Celerity
{
struct Sprite2dUvAnimationLoadingState final
{
    Memory::UniqueString assetId;

    std::atomic<AssetState> state {AssetState::LOADING};

    bool valid = true;

    Sprite2dUvAnimationAsset asset;

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

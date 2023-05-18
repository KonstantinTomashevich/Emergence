#pragma once

#include <Celerity/Asset/Asset.hpp>
#include <Celerity/Asset/LoadingSharedState.hpp>
#include <Celerity/Asset/Render/2d/Sprite2dUvAnimation.hpp>

#include <StandardLayout/Mapping.hpp>

namespace Emergence::Celerity
{
class Sprite2dUvAnimationLoadingSharedState final : public LoadingSharedState<Sprite2dUvAnimationLoadingSharedState>
{
public:
    static constexpr const char *ALLOCATION_GROUP_NAME = "Sprite2dUvAnimationLoading";

    Sprite2dUvAnimationAsset asset;
};

struct Sprite2dUvAnimationLoadingState final
{
    Memory::UniqueString assetId;

    Handling::Handle<Sprite2dUvAnimationLoadingSharedState> sharedState {new Sprite2dUvAnimationLoadingSharedState};

    struct Reflection final
    {
        StandardLayout::FieldId assetId;
        StandardLayout::Mapping mapping;
    };

    static const Reflection &Reflect () noexcept;
};
} // namespace Emergence::Celerity

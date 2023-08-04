#pragma once

#include <CelerityRender2dModelApi.hpp>

#include <Celerity/Asset/Asset.hpp>
#include <Celerity/Asset/Render/2d/Sprite2dUvAnimation.hpp>
#include <Celerity/Standard/ContextEscape.hpp>

#include <StandardLayout/Mapping.hpp>

namespace Emergence::Celerity
{
/// \brief Shared loading state for sprite 2d animations.
/// \details Intended for use only inside CelerityRender2dLogic, therefore undocumented.
class CelerityRender2dModelApi Sprite2dUvAnimationLoadingSharedState final
    : public ContextEscape<Sprite2dUvAnimationLoadingSharedState>
{
public:
    static constexpr const char *ALLOCATION_GROUP_NAME = "Sprite2dUvAnimationLoading";

    /// \brief Asset loading state is used as return value for processed asset loading jobs.
    std::atomic<AssetState> state {AssetState::LOADING};

    Sprite2dUvAnimationAsset asset;
};

/// \brief Loading state for sprite 2d animations.
/// \details Intended for use only inside CelerityRender2dLogic, therefore undocumented.
struct CelerityRender2dModelApi Sprite2dUvAnimationLoadingState final
{
    Memory::UniqueString assetId;

    Handling::Handle<Sprite2dUvAnimationLoadingSharedState> sharedState {new Sprite2dUvAnimationLoadingSharedState};

    struct CelerityRender2dModelApi Reflection final
    {
        StandardLayout::FieldId assetId;
        StandardLayout::Mapping mapping;
    };

    static const Reflection &Reflect () noexcept;
};
} // namespace Emergence::Celerity

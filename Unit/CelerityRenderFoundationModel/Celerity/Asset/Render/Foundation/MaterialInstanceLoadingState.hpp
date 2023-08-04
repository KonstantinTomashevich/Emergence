#pragma once

#include <CelerityRenderFoundationModelApi.hpp>

#include <Celerity/Asset/Asset.hpp>
#include <Celerity/Asset/Render/Foundation/MaterialInstance.hpp>
#include <Celerity/Standard/ContextEscape.hpp>

#include <StandardLayout/Mapping.hpp>

namespace Emergence::Celerity
{
/// \brief Shared state for material instance asset loading.
/// \details Intended for use only inside CelerityRenderFoundationLogic, therefore undocumented.
class CelerityRenderFoundationModelApi MaterialInstanceLoadingSharedState final
    : public ContextEscape<MaterialInstanceLoadingSharedState>
{
public:
    static constexpr const char *ALLOCATION_GROUP_NAME = "MaterialInstanceLoading";

    /// \brief Asset loading state is used as return value for processed asset loading jobs.
    std::atomic<AssetState> state {AssetState::LOADING};

    MaterialInstanceAsset asset;
};

/// \brief Loading state for material instance asset.
/// \details Intended for use only inside CelerityRenderFoundationLogic, therefore undocumented.
struct CelerityRenderFoundationModelApi MaterialInstanceLoadingState final
{
    Memory::UniqueString assetId;

    Memory::UniqueString parentId;

    Handling::Handle<MaterialInstanceLoadingSharedState> sharedState {new MaterialInstanceLoadingSharedState};

    struct CelerityRenderFoundationModelApi Reflection final
    {
        StandardLayout::FieldId assetId;
        StandardLayout::FieldId parentId;
        StandardLayout::Mapping mapping;
    };

    static const Reflection &Reflect () noexcept;
};
} // namespace Emergence::Celerity

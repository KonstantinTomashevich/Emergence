#pragma once

#include <CelerityRenderFoundationModelApi.hpp>

#include <Celerity/Asset/Asset.hpp>
#include <Celerity/Asset/LoadingSharedState.hpp>
#include <Celerity/Asset/Render/Foundation/MaterialInstance.hpp>

#include <StandardLayout/Mapping.hpp>

namespace Emergence::Celerity
{
/// \brief Shared state for material instance asset loading.
/// \details Intended for use only inside CelerityRenderFoundationLogic, therefore undocumented.
class CelerityRenderFoundationModelApi MaterialInstanceLoadingSharedState final : public LoadingSharedState<MaterialInstanceLoadingSharedState>
{
public:
    static constexpr const char *ALLOCATION_GROUP_NAME = "MaterialInstanceLoading";

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

#pragma once

#include <CelerityRenderFoundationModelApi.hpp>

#include <Celerity/Asset/Asset.hpp>
#include <Celerity/Asset/LoadingSharedState.hpp>
#include <Celerity/Asset/Render/Foundation/Material.hpp>

#include <StandardLayout/Mapping.hpp>

namespace Emergence::Celerity
{
/// \brief Shared state for material asset loading.
/// \details Intended for use only inside CelerityRenderFoundationLogic, therefore undocumented.
class CelerityRenderFoundationModelApi MaterialLoadingSharedState final : public LoadingSharedState<MaterialLoadingSharedState>
{
public:
    static constexpr const char *ALLOCATION_GROUP_NAME = "MaterialLoading";

    EMERGENCE_STATIONARY_DATA_TYPE (MaterialLoadingSharedState);

    MaterialAsset asset;

    Memory::Heap shaderDataHeap {GetAllocationGroup ()};

    std::uint64_t vertexSharedSize = 0u;

    std::uint8_t *vertexShaderData = nullptr;

    std::uint64_t fragmentSharedSize = 0u;

    std::uint8_t *fragmentShaderData = nullptr;
};

/// \brief Loading state for material asset.
/// \details Intended for use only inside CelerityRenderFoundationLogic, therefore undocumented.
struct CelerityRenderFoundationModelApi MaterialLoadingState final
{
    Memory::UniqueString assetId;

    Handling::Handle<MaterialLoadingSharedState> sharedState {new MaterialLoadingSharedState};

    struct CelerityRenderFoundationModelApi Reflection final
    {
        StandardLayout::FieldId assetId;
        StandardLayout::Mapping mapping;
    };

    static const Reflection &Reflect () noexcept;
};
} // namespace Emergence::Celerity

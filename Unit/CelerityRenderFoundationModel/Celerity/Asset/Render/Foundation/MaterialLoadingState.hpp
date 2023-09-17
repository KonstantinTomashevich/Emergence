#pragma once

#include <CelerityRenderFoundationModelApi.hpp>

#include <Celerity/Asset/Asset.hpp>
#include <Celerity/Asset/Render/Foundation/Material.hpp>
#include <Celerity/Standard/ContextEscape.hpp>

#include <StandardLayout/Mapping.hpp>

namespace Emergence::Celerity
{
/// \brief Shared state for material asset loading.
/// \details Intended for use only inside CelerityRenderFoundationLogic, therefore undocumented.
class CelerityRenderFoundationModelApi MaterialLoadingSharedState final
    : public ContextEscape<MaterialLoadingSharedState>
{
public:
    static constexpr const char *ALLOCATION_GROUP_NAME = "MaterialLoading";

    EMERGENCE_STATIONARY_DATA_TYPE (MaterialLoadingSharedState);

    /// \brief Asset loading state is used as return value for processed asset loading jobs.
    std::atomic<AssetState> state {AssetState::LOADING};

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

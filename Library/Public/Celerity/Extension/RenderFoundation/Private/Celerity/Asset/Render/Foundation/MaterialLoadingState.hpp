#pragma once

#include <Celerity/Asset/Asset.hpp>
#include <Celerity/Asset/LoadingSharedState.hpp>
#include <Celerity/Asset/Render/Foundation/Material.hpp>

#include <StandardLayout/Mapping.hpp>

namespace Emergence::Celerity
{
class MaterialLoadingSharedState final : public LoadingSharedState<MaterialLoadingSharedState>
{
public:
    static constexpr const char *ALLOCATION_GROUP_NAME = "MaterialLoading";

    EMERGENCE_STATIONARY_DATA_TYPE (MaterialLoadingSharedState);

    MaterialAsset asset;

    Memory::Heap shaderDataHeap {GetAllocationGroup ()};

    uint64_t vertexSharedSize = 0u;

    uint8_t *vertexShaderData = nullptr;

    uint64_t fragmentSharedSize = 0u;

    uint8_t *fragmentShaderData = nullptr;
};

struct MaterialLoadingState final
{
    Memory::UniqueString assetId;

    Handling::Handle<MaterialLoadingSharedState> sharedState {new MaterialLoadingSharedState};

    struct Reflection final
    {
        StandardLayout::FieldId assetId;
        StandardLayout::Mapping mapping;
    };

    static const Reflection &Reflect () noexcept;
};
} // namespace Emergence::Celerity

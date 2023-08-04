#pragma once

#include <CelerityRenderFoundationModelApi.hpp>

#include <Celerity/Asset/Asset.hpp>
#include <Celerity/Asset/Render/Foundation/Texture.hpp>
#include <Celerity/Standard/ContextEscape.hpp>

#include <StandardLayout/Mapping.hpp>

namespace Emergence::Celerity
{
/// \brief Shared state for texture asset loading.
/// \details Intended for use only inside CelerityRenderFoundationLogic, therefore undocumented.
class CelerityRenderFoundationModelApi TextureLoadingSharedState final : public ContextEscape<TextureLoadingSharedState>
{
public:
    static constexpr const char *ALLOCATION_GROUP_NAME = "TextureLoading";

    EMERGENCE_STATIONARY_DATA_TYPE (TextureLoadingSharedState);

    /// \brief Asset loading state is used as return value for processed asset loading jobs.
    std::atomic<AssetState> state {AssetState::LOADING};

    TextureAsset asset;

    Memory::Heap textureDataHeap {GetHeap ().GetAllocationGroup ()};

    std::uint64_t textureDataSize = 0u;

    std::uint8_t *textureData = nullptr;
};

/// \brief Loading state for texture asset.
/// \details Intended for use only inside CelerityRenderFoundationLogic, therefore undocumented.
struct CelerityRenderFoundationModelApi TextureLoadingState final
{
    Memory::UniqueString assetId;

    Handling::Handle<TextureLoadingSharedState> sharedState {new TextureLoadingSharedState};

    struct CelerityRenderFoundationModelApi Reflection final
    {
        StandardLayout::FieldId assetId;
        StandardLayout::Mapping mapping;
    };

    static const Reflection &Reflect () noexcept;
};
} // namespace Emergence::Celerity

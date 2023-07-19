#pragma once

#include <Celerity/Asset/Asset.hpp>
#include <Celerity/Asset/LoadingSharedState.hpp>
#include <Celerity/Asset/Render/Foundation/Texture.hpp>

#include <StandardLayout/Mapping.hpp>

namespace Emergence::Celerity
{
class TextureLoadingSharedState final : public LoadingSharedState<TextureLoadingSharedState>
{
public:
    static constexpr const char *ALLOCATION_GROUP_NAME = "TextureLoading";

    EMERGENCE_STATIONARY_DATA_TYPE (TextureLoadingSharedState);

    TextureAsset asset;

    Memory::Heap textureDataHeap {GetHeap ().GetAllocationGroup ()};

    std::uint64_t textureDataSize = 0u;

    std::uint8_t *textureData = nullptr;
};

struct TextureLoadingState final
{
    Memory::UniqueString assetId;

    Handling::Handle<TextureLoadingSharedState> sharedState {new TextureLoadingSharedState};

    struct Reflection final
    {
        StandardLayout::FieldId assetId;
        StandardLayout::Mapping mapping;
    };

    static const Reflection &Reflect () noexcept;
};
} // namespace Emergence::Celerity
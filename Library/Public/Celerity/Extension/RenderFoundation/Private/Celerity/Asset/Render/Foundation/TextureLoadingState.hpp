#pragma once

#include <Celerity/Asset/Asset.hpp>
#include <Celerity/Asset/LoadingSharedState.hpp>

#include <Render/Backend/Texture.hpp>

#include <StandardLayout/Mapping.hpp>

namespace Emergence::Celerity
{
class TextureLoadingSharedState final : public LoadingSharedState<TextureLoadingSharedState>
{
public:
    static constexpr const char *ALLOCATION_GROUP_NAME = "TextureLoading";

    EMERGENCE_STATIONARY_DATA_TYPE (TextureLoadingSharedState);

    Render::Backend::TextureSettings settings;

    Memory::Heap textureDataHeap {GetHeap ().GetAllocationGroup ()};

    uint64_t textureDataSize = 0u;

    uint8_t *textureData = nullptr;
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

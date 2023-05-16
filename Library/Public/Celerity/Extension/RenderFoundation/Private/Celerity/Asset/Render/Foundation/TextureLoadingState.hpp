#pragma once

#include <Celerity/Asset/Asset.hpp>

#include <Render/Backend/Texture.hpp>

#include <StandardLayout/Mapping.hpp>

namespace Emergence::Celerity
{
struct TextureLoadingState final
{
    EMERGENCE_STATIONARY_DATA_TYPE (TextureLoadingState);

    Memory::UniqueString assetId;

    std::atomic<AssetState> state {AssetState::LOADING};

    bool valid = true;

    Render::Backend::TextureSettings settings;

    Memory::Heap textureDataHeap {Memory::Profiler::AllocationGroup::Top ()};

    uint64_t textureDataSize = 0u;

    uint8_t *textureData = nullptr;

    struct Reflection final
    {
        StandardLayout::FieldId assetId;
        StandardLayout::FieldId valid;
        StandardLayout::FieldId settings;
        StandardLayout::FieldId textureDataSize;
        StandardLayout::Mapping mapping;
    };

    static const Reflection &Reflect () noexcept;
};
} // namespace Emergence::Celerity

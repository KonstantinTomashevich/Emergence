#include <Celerity/Asset/Render/Foundation/TextureLoadingState.hpp>

#include <StandardLayout/MappingRegistration.hpp>

namespace Emergence::Celerity
{
TextureLoadingSharedState::TextureLoadingSharedState () noexcept = default;

TextureLoadingSharedState::~TextureLoadingSharedState () noexcept
{
    if (textureData)
    {
        textureDataHeap.Release (textureData, textureDataSize);
    }
}

const TextureLoadingState::Reflection &TextureLoadingState::Reflect () noexcept
{
    static Reflection reflection = [] ()
    {
        EMERGENCE_MAPPING_REGISTRATION_BEGIN (TextureLoadingState);
        EMERGENCE_MAPPING_REGISTER_REGULAR (assetId);
        EMERGENCE_MAPPING_REGISTRATION_END ();
    }();

    return reflection;
}
} // namespace Emergence::Celerity

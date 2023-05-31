#include <Celerity/Asset/Render/Foundation/Texture.hpp>

#include <StandardLayout/MappingRegistration.hpp>

namespace Emergence::Celerity
{
const TextureAsset::Reflection &TextureAsset::Reflect () noexcept
{
    static const Reflection reflection = [] ()
    {
        EMERGENCE_MAPPING_REGISTRATION_BEGIN (TextureAsset);
        EMERGENCE_MAPPING_REGISTER_REGULAR (textureId);
        EMERGENCE_MAPPING_REGISTER_REGULAR (settings);
        EMERGENCE_MAPPING_REGISTRATION_END ();
    }();

    return reflection;
}
} // namespace Emergence::Celerity

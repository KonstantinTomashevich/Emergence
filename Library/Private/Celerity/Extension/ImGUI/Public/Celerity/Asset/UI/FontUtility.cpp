#include <Assert/Assert.hpp>

#include <Celerity/Asset/UI/FontUtility.hpp>

namespace Emergence::Celerity
{
Render::Backend::Texture BakeFontAtlas (ImFontAtlas *_atlas)
{
    unsigned char *textureData;
    int textureWidth;
    int textureHeight;
    _atlas->GetTexDataAsRGBA32 (&textureData, &textureWidth, &textureHeight);
    EMERGENCE_ASSERT (textureData);

    Render::Backend::Texture texture (textureData, textureWidth, textureHeight, {});
    const Render::Backend::TextureId atlasTextureId = texture.GetId ();
    static_assert (sizeof (Render::Backend::TextureId) == sizeof (ImTextureID));
    BEGIN_MUTING_STRING_ALIASING_WARNINGS
    _atlas->SetTexID (*reinterpret_cast<const ImTextureID *> (&atlasTextureId));
    END_MUTING_WARNINGS
    return texture;
}
} // namespace Emergence::Celerity

#include <CelerityRenderFoundationModelApi.hpp>

#include <Render/Backend/Texture.hpp>

namespace Emergence::Celerity
{
/// \brief Content of texture asset file that contains info about texture source and texture settings.
struct CelerityRenderFoundationModelApi TextureAsset final
{
    /// \brief Id of file with third party format that contains texture data, like "Crate.png".
    Memory::UniqueString textureId;

    /// \brief Settings for loading this texture.
    Render::Backend::TextureSettings settings;

    struct CelerityRenderFoundationModelApi Reflection final
    {
        StandardLayout::FieldId textureId;
        StandardLayout::FieldId settings;
        StandardLayout::Mapping mapping;
    };

    static const Reflection &Reflect () noexcept;
};
} // namespace Emergence::Celerity

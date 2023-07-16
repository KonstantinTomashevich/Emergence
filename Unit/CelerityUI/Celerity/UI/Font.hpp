#pragma once

#include <Celerity/Standard/UniqueId.hpp>

#include <Render/Backend/Texture.hpp>

#include <StandardLayout/Mapping.hpp>

namespace Emergence::Celerity
{
/// \brief Used to separate font name from font size in font asset ids.
/// \details See Font::assetId.
constexpr char FONT_SIZE_SEPARATOR = '#';

/// \brief Represents loaded font asset with specific font size.
struct Font final
{
    EMERGENCE_STATIONARY_DATA_TYPE (Font);

    /// \brief Id used to bind to Asset instance.
    /// \details Must follow the format `{FontName}{FONT_SIZE_SEPARATOR}{FontSize}`, for example `F_DroidSans#14`.
    Memory::UniqueString assetId;

    /// \brief Texture that contains loaded font atlas.
    Render::Backend::Texture atlasTexture = Render::Backend::Texture::CreateInvalid ();

    /// \brief Implementation specific handle, if any.
    void *nativeHandle = nullptr;

    struct Reflection final
    {
        StandardLayout::FieldId assetId;
        StandardLayout::FieldId size;
        StandardLayout::FieldId nativeHandle;
        StandardLayout::Mapping mapping;
    };

    static const Reflection &Reflect () noexcept;
};
} // namespace Emergence::Celerity

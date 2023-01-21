#pragma once

#include <Celerity/Standard/UniqueId.hpp>

#include <Render/Backend/Texture.hpp>

#include <StandardLayout/Mapping.hpp>

namespace Emergence::Celerity
{
/// \brief Attachment to Viewport that directs render commands for UI rendering into this viewport.
/// \invariant Only one render pass attachment per Viewport is supported!
struct UIRenderPass final
{
    EMERGENCE_STATIONARY_DATA_TYPE (UIRenderPass);

    /// \brief Name of the viewport to which this pass is attached.
    Memory::UniqueString name;

    /// \brief Id of the default style that will be applied to the top level nodes unless they specified their style.
    Memory::UniqueString defaultStyleId;

    /// \brief Native implementation context, if any.
    void *nativeContext = nullptr;

    /// \brief Texture for default font atlas.
    /// \details Most UI frameworks provide inbuilt default font, that will be used if no other
    ///          font is selected. This texture is used to upload this font atlas to GPU.
    Render::Backend::Texture defaultFontTexture;

    struct Reflection final
    {
        StandardLayout::FieldId name;
        StandardLayout::FieldId defaultStyleId;
        StandardLayout::FieldId nativeContext;
        StandardLayout::Mapping mapping;
    };

    static const Reflection &Reflect () noexcept;
};
} // namespace Emergence::Celerity

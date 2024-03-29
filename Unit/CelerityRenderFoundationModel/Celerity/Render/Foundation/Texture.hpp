#pragma once

#include <CelerityRenderFoundationModelApi.hpp>

#include <Memory/UniqueString.hpp>

#include <Render/Backend/Texture.hpp>

#include <StandardLayout/Mapping.hpp>

namespace Emergence::Celerity
{
/// \brief Represents successfully loaded texture asset.
struct CelerityRenderFoundationModelApi Texture final
{
    /// \brief Id used to bind to Asset instance.
    Memory::UniqueString assetId;

    /// \brief Underlying implementation object.
    Render::Backend::Texture texture = Render::Backend::Texture::CreateInvalid ();

    struct CelerityRenderFoundationModelApi Reflection final
    {
        StandardLayout::FieldId assetId;
        StandardLayout::Mapping mapping;
    };

    static const Reflection &Reflect () noexcept;
};
} // namespace Emergence::Celerity

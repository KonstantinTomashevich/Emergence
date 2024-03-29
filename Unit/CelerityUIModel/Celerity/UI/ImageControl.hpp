#pragma once

#include <CelerityUIModelApi.hpp>

#include <Celerity/Standard/UniqueId.hpp>

#include <Math/AxisAlignedBox2d.hpp>

#include <StandardLayout/Mapping.hpp>

namespace Emergence::Celerity
{
/// \brief Control that renders given texture.
struct CelerityUIModelApi ImageControl final
{
    /// \brief Id of an UINode, to which this control is attached.
    UniqueId nodeId = INVALID_UNIQUE_ID;

    /// \brief Width of this control.
    std::uint32_t width = 0u;

    /// \brief Height of this control.
    std::uint32_t height = 0u;

    /// \brief Id of a texture that should be rendered.
    Memory::UniqueString textureId;

    /// \brief UV-coordinates for rendering texture.
    Math::AxisAlignedBox2d uv;

    struct CelerityUIModelApi Reflection final
    {
        StandardLayout::FieldId nodeId;
        StandardLayout::FieldId width;
        StandardLayout::FieldId height;

        StandardLayout::FieldId textureId;
        StandardLayout::FieldId uv;
        StandardLayout::Mapping mapping;
    };

    static const Reflection &Reflect () noexcept;
};
} // namespace Emergence::Celerity

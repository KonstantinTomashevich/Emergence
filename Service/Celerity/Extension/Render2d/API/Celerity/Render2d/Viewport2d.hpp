#pragma once

#include <Celerity/Standard/UniqueId.hpp>

#include <StandardLayout/Mapping.hpp>

namespace Emergence::Celerity
{
/// \brief Configures output surface for 2d rendering. Having multiple viewports is supported.
struct Viewport2d final
{
    Viewport2d () noexcept;

    /// \invariant Must be unique among other viewports.
    Memory::UniqueString name;

    /// \brief Id of the camera object that will be used for rendering.
    UniqueId cameraObjectId = INVALID_UNIQUE_ID;

    /// \brief Used to sort viewports in case of overlap.
    std::uint16_t sortIndex = 0u;

    /// \brief Native identifier used to pass data to rendering backend.
    /// \details Generated automatically, do not override.
    std::uint16_t nativeId = 0u;

    /// \brief Leftmost coordinate of the viewport in window coordinates. Zero is the window left border.
    std::uint32_t x = 0u;

    /// \brief Topmost coordinate of the viewport in window coordinates. Zero is the window top border.
    std::uint32_t y = 0u;

    /// \brief Width of the viewport in window coordinates.
    std::uint32_t width = 0u;

    /// \brief Height of the viewport in window coordinates.
    std::uint32_t height = 0u;

    /// \brief Clear color for this viewport.
    /// \details If color is equal to 0x00000000, then viewport is processed as fully transparent and color clearing
    ///          is disabled at all. This behaviour is required for things that need to overlap other things, like UI.
    uint32_t clearColor = 0x000000FF;

    struct Reflection final
    {
        StandardLayout::FieldId name;
        StandardLayout::FieldId cameraObjectId;
        StandardLayout::FieldId sortIndex;
        StandardLayout::FieldId nativeId;
        StandardLayout::FieldId x;
        StandardLayout::FieldId y;
        StandardLayout::FieldId width;
        StandardLayout::FieldId height;
        StandardLayout::FieldId clearColor;
        StandardLayout::Mapping mapping;
    };

    static const Reflection &Reflect () noexcept;
};
} // namespace Emergence::Celerity

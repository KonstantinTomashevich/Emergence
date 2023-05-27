#pragma once

#include <Celerity/Standard/UniqueId.hpp>

#include <Render/Backend/Viewport.hpp>

#include <StandardLayout/Mapping.hpp>

namespace Emergence::Celerity
{
// TODO: Rework viewports into fully fledged render target system?

/// \brief Configures output surface for rendering. Having multiple viewports is supported.
struct Viewport final
{
    /// \brief Readable name for this viewport.
    /// \invariant Must be unique among other viewports.
    Memory::UniqueString name;

    /// \brief Used to sort viewports in case of overlap.
    std::uint16_t sortIndex = 0u;

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
    std::uint32_t clearColor = 0x000000FF;

    /// \brief Defines how submitted render commands are sorted by viewport.
    Render::Backend::ViewportSortMode sortMode = Render::Backend::ViewportSortMode::SEQUENTIAL;

    /// \brief Underlying implementation object.
    /// \details Field is mutable, because render backend objects are technically handles and by modifying them
    ///          we work with underlying implementation that operates under different read-write ruleset.
    mutable Render::Backend::Viewport viewport;

    struct Reflection final
    {
        StandardLayout::FieldId name;
        StandardLayout::FieldId sortIndex;
        StandardLayout::FieldId x;
        StandardLayout::FieldId y;
        StandardLayout::FieldId width;
        StandardLayout::FieldId height;
        StandardLayout::FieldId clearColor;
        StandardLayout::FieldId sortMode;
        StandardLayout::Mapping mapping;
    };

    static const Reflection &Reflect () noexcept;
};
} // namespace Emergence::Celerity

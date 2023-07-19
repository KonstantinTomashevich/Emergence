#pragma once

#include <CelerityRender2dApi.hpp>

#include <Celerity/Standard/UniqueId.hpp>

#include <StandardLayout/Mapping.hpp>

namespace Emergence::Celerity
{
/// \brief Attachment to Viewport that directs render commands for 2d rendering into this viewport.
/// \invariant Only one render pass attachment per Viewport is supported!
struct CelerityRender2dApi World2dRenderPass final
{
    /// \brief Name of the viewport to which this pass is attached.
    Memory::UniqueString name;

    /// \brief Id of the camera object that will be used for rendering.
    UniqueId cameraObjectId = INVALID_UNIQUE_ID;

    struct CelerityRender2dApi Reflection final
    {
        StandardLayout::FieldId name;
        StandardLayout::FieldId cameraObjectId;
        StandardLayout::Mapping mapping;
    };

    static const Reflection &Reflect () noexcept;
};
} // namespace Emergence::Celerity

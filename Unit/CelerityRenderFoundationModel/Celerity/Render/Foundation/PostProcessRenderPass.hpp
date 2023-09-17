#pragma once

#include <CelerityRenderFoundationModelApi.hpp>

#include <StandardLayout/Mapping.hpp>

namespace Emergence::Celerity
{
/// \brief Attachment to Viewport that directs render commands for post processing into this viewport.
/// \invariant Only one render pass attachment per Viewport is supported!
/// \details Post processing is essentially rendering a full screen rectangle with given material instance.
struct CelerityRenderFoundationModelApi PostProcessRenderPass final
{
    /// \brief Name of the viewport to which this pass is attached.
    Memory::UniqueString name;

    /// \brief Material instance that is used for post processing.
    Memory::UniqueString materialInstanceId;

    struct CelerityRenderFoundationModelApi Reflection final
    {
        StandardLayout::FieldId name;
        StandardLayout::FieldId materialInstanceId;
        StandardLayout::Mapping mapping;
    };

    static const Reflection &Reflect () noexcept;
};
} // namespace Emergence::Celerity

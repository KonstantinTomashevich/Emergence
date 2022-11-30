#pragma once

#include <Celerity/Standard/UniqueId.hpp>

#include <Render/Backend/Renderer.hpp>

#include <StandardLayout/Mapping.hpp>

namespace Emergence::Celerity
{
/// \brief Stores shared RenderBackend objects.
struct RenderFoundationSingleton final
{
    /// \brief Backend implementation object.
    /// \details Field is mutable, because render backend objects are technically handles and by modifying them
    ///          we work with underlying implementation that operates under different read-write ruleset.
    mutable Render::Backend::Renderer renderer;

    struct Reflection final
    {
        StandardLayout::Mapping mapping;
    };

    static const Reflection &Reflect () noexcept;
};
} // namespace Emergence::Celerity

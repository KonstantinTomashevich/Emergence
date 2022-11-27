#pragma once

#include <Celerity/Standard/UniqueId.hpp>

#include <Render/Backend/Renderer.hpp>

#include <StandardLayout/Mapping.hpp>

namespace Emergence::Celerity
{
/// \brief Contains common parameters of world 2d rendering.
struct Render2dSingleton final
{
    /// \brief Underlying implementation object.
    Render::Backend::Renderer renderer;

    /// \invariant Do not access directly, use ::GenerateSprite2dId.
    std::atomic_unsigned_lock_free sprite2dIdCounter = 0u;

    /// \details Intentionally const to allow simultaneous access from multiple tasks.
    uintptr_t GenerateSprite2dId () const noexcept;

    struct Reflection final
    {
        StandardLayout::Mapping mapping;
    };

    static const Reflection &Reflect () noexcept;
};
} // namespace Emergence::Celerity

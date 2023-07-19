#pragma once

#include <CelerityRender2dApi.hpp>

#include <atomic>

#include <Celerity/Standard/UniqueId.hpp>

#include <Render/Backend/Renderer.hpp>

#include <StandardLayout/Mapping.hpp>

namespace Emergence::Celerity
{
/// \brief Contains common parameters of world 2d rendering.
struct CelerityRender2dApi Render2dSingleton final
{
    /// \invariant Do not access directly, use ::GenerateSprite2dId.
    std::atomic_uintptr_t sprite2dIdCounter = 0u;

    /// \invariant Do not access directly, use ::GenerateDebugShape2dId.
    std::atomic_uintptr_t debugShape2dIdCounter = 0u;

    /// \details Intentionally const to allow simultaneous access from multiple tasks.
    [[nodiscard]] std::uintptr_t GenerateSprite2dId () const noexcept;

    /// \details Intentionally const to allow simultaneous access from multiple tasks.
    [[nodiscard]] std::uintptr_t GenerateDebugShape2dId () const noexcept;

    struct CelerityRender2dApi Reflection final
    {
        StandardLayout::Mapping mapping;
    };

    static const Reflection &Reflect () noexcept;
};
} // namespace Emergence::Celerity

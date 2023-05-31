#pragma once

#include <atomic>

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

    /// \invariant Do not access directly, use ::GenerateRuntimeMaterialInstanceId.
    std::atomic_uintptr_t runtimeMaterialInstanceIdCounter = 0u;

    /// \details Intentionally const to allow simultaneous access from multiple tasks.
    [[nodiscard]] Memory::UniqueString GenerateRuntimeMaterialInstanceId (
        Memory::UniqueString _materialInstanceId) const noexcept;

    struct Reflection final
    {
        StandardLayout::Mapping mapping;
    };

    static const Reflection &Reflect () noexcept;
};
} // namespace Emergence::Celerity

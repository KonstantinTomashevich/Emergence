#pragma once

#include <CelerityUIModelApi.hpp>

#include <atomic>

#include <Celerity/Standard/UniqueId.hpp>

#include <StandardLayout/Mapping.hpp>

namespace Emergence::Celerity
{
/// \brief Contains common parameters for UI extension.
struct CelerityUIModelApi UISingleton final
{
    /// \brief Atomic counter for generating unique ids for UI nodes.
    /// \invariant Do not access directly, use ::GenerateNodeId.
    std::atomic_uintptr_t nodeIdCounter = 0u;

    /// \brief Generates new unique id for a UI node.
    /// \details Intentionally const to allow simultaneous access from multiple tasks.
    [[nodiscard]] std::uintptr_t GenerateNodeId () const noexcept;

    struct CelerityUIModelApi Reflection final
    {
        StandardLayout::Mapping mapping;
    };

    static const Reflection &Reflect () noexcept;
};
} // namespace Emergence::Celerity

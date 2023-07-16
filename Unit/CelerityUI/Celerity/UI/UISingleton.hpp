#pragma once

#include <atomic>

#include <Celerity/Standard/UniqueId.hpp>

#include <StandardLayout/Mapping.hpp>

namespace Emergence::Celerity
{
/// \brief Contains common parameters for UI extension.
struct UISingleton final
{
    /// \brief Whether Emergence MemoryProfiler should be enabled for UI allocations.
    /// \details Enabling MemoryProfiler can increase memory usage or decrease performance.
    /// \invariant Can not be changed after first normal update.
    bool enableMemoryProfiling = false;

    /// \brief Atomic counter for generating unique ids for UI nodes.
    /// \invariant Do not access directly, use ::GenerateNodeId.
    std::atomic_uintptr_t nodeIdCounter = 0u;

    /// \brief Generates new unique id for a UI node.
    /// \details Intentionally const to allow simultaneous access from multiple tasks.
    [[nodiscard]] std::uintptr_t GenerateNodeId () const noexcept;

    struct Reflection final
    {
        StandardLayout::FieldId enableMemoryProfiling;
        StandardLayout::Mapping mapping;
    };

    static const Reflection &Reflect () noexcept;
};
} // namespace Emergence::Celerity

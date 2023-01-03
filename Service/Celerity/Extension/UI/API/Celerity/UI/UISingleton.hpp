#pragma once

#include <Celerity/Standard/UniqueId.hpp>

#include <StandardLayout/Mapping.hpp>

namespace Emergence::Celerity
{
struct UISingleton final
{
    /// \brief Whether Emergence MemoryProfiler should be enabled for UI allocations.
    /// \details Enabling MemoryProfiler can increase memory usage or decrease performance.
    /// \invariant Can not be changed after first normal update.
    bool enableMemoryProfiling = false;

    /// \brief Atomic counter for generating unique ids for UI nodes.
    /// \invariant Do not access directly, use ::GenerateNodeId.
    std::atomic_unsigned_lock_free nodeIdCounter = 0u;

    /// \brief Generates new unique id for a UI node.
    /// \details Intentionally const to allow simultaneous access from multiple tasks.
    [[nodiscard]] uintptr_t GenerateNodeId () const noexcept;

    struct Reflection final
    {
        StandardLayout::FieldId enableMemoryProfiling;
        StandardLayout::Mapping mapping;
    };

    static const Reflection &Reflect () noexcept;
};
} // namespace Emergence::Celerity

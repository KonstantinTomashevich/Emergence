#pragma once

#include <Celerity/Standard/UniqueId.hpp>

#include <StandardLayout/Mapping.hpp>

namespace Emergence::Celerity
{
struct UISingleton final
{
    /// \brief Atomic counter for generating unique ids for UI nodes.
    /// \invariant Do not access directly, use ::GenerateNodeId.
    std::atomic_unsigned_lock_free nodeIdCounter = 0u;

    /// \brief Generates new unique id for a UI node.
    /// \details Intentionally const to allow simultaneous access from multiple tasks.
    uintptr_t GenerateNodeId () const noexcept;

    struct Reflection final
    {
        StandardLayout::Mapping mapping;
    };

    static const Reflection &Reflect () noexcept;
};
} // namespace Emergence::Celerity

#pragma once

#include <atomic>
#include <cstdint>

#include <StandardLayout/Mapping.hpp>

namespace Emergence::Celerity
{
/// \brief Singleton for world<->tasks communication and global utility like UID generation.
/// \warning This singleton is modified by World outside of pipeline execution,
///          therefore OnChange events do not work with it.
struct WorldSingleton final
{
    /// \brief Indicates whether current normal update was separated from previous one by one or more fixed updates.
    /// \warning Access outside of normal update routine leads to undefined behaviour.
    bool fixedUpdateHappened = false;

    /// \invariant Do not access directly, use ::GenerateUID.
    std::atomic_unsigned_lock_free uidCounter = 0u;

    /// \details Intentionally const to allow simultaneous access from multiple tasks.
    uintptr_t GenerateUID () const noexcept;

    struct Reflection final
    {
        StandardLayout::FieldId fixedUpdateHappened;
        StandardLayout::Mapping mapping;
    };

    static const Reflection &Reflect () noexcept;
};
} // namespace Emergence::Celerity

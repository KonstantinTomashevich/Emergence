#pragma once

#include <Celerity/Standard/UniqueId.hpp>

#include <StandardLayout/Mapping.hpp>

struct PlayerInfoSingleton final
{
    Emergence::Celerity::UniqueId localPlayerUid = 0u;

    /// \invariant Do not access directly, use ::GeneratePlayerId.
    std::atomic_unsigned_lock_free playerUidCounter = 1u;

    /// \details Intentionally const to allow simultaneous access from multiple tasks.
    uintptr_t GeneratePlayerId () const noexcept;

    struct Reflection
    {
        Emergence::StandardLayout::FieldId localPlayerUid;
        Emergence::StandardLayout::Mapping mapping;
    };

    static const Reflection &Reflect () noexcept;
};

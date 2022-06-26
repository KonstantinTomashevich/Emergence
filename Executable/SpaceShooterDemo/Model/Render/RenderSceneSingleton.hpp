#pragma once

#include <API/Common/Shortcuts.hpp>

#include <Celerity/Standard/UniqueId.hpp>

#include <StandardLayout/Mapping.hpp>

struct RenderSceneSingleton final
{
    Emergence::Celerity::UniqueId cameraObjectId = Emergence::Celerity::INVALID_UNIQUE_ID;

    /// \invariant Do not access directly, use ::GenerateLightId.
    std::atomic_unsigned_lock_free lightIdCounter = 0u;

    /// \invariant Do not access directly, use ::GenerateEffectId.
    std::atomic_unsigned_lock_free effectIdCounter = 0u;

    /// \invariant Do not access directly, use ::GenerateModelId.
    std::atomic_unsigned_lock_free modelIdCounter = 0u;

    /// \details Intentionally const to allow simultaneous access from multiple tasks.
    uintptr_t GenerateLightId () const noexcept;

    /// \details Intentionally const to allow simultaneous access from multiple tasks.
    uintptr_t GenerateModelId () const noexcept;

    /// \details Intentionally const to allow simultaneous access from multiple tasks.
    uintptr_t GenerateEffectId () const noexcept;

    struct Reflection final
    {
        Emergence::StandardLayout::FieldId cameraObjectId;
        Emergence::StandardLayout::Mapping mapping;
    };

    static const Reflection &Reflect () noexcept;
};

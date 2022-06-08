#pragma once

#include <API/Common/Shortcuts.hpp>

#include <Celerity/Standard/UniqueId.hpp>

#include <StandardLayout/Mapping.hpp>

struct RenderSceneSingleton final
{
    Emergence::Celerity::UniqueId cameraObjectId = Emergence::Celerity::INVALID_UNIQUE_ID;

    /// \invariant Do not access directly, use ::GenerateLightUID.
    std::atomic_unsigned_lock_free lightUIDCounter = 0u;

    /// \invariant Do not access directly, use ::GenerateModelUID.
    std::atomic_unsigned_lock_free modelUIDCounter = 0u;

    /// \details Intentionally const to allow simultaneous access from multiple tasks.
    uintptr_t GenerateLightUID () const noexcept;

    /// \details Intentionally const to allow simultaneous access from multiple tasks.
    uintptr_t GenerateModelUID () const noexcept;

    struct Reflection final
    {
        Emergence::StandardLayout::FieldId cameraObjectId;
        Emergence::StandardLayout::Mapping mapping;
    };

    static const Reflection &Reflect () noexcept;
};

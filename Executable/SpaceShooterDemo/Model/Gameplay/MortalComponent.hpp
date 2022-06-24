#pragma once

#include <Celerity/Standard/UniqueId.hpp>

#include <StandardLayout/Mapping.hpp>

struct MortalComponent final
{
    Emergence::Celerity::UniqueId objectId = Emergence::Celerity::INVALID_UNIQUE_ID;

    float health = 1.0f;

    float maxHealth = 1.0f;

    /// \details Timer for transform removal after death.
    uint64_t removeAfterNs = std::numeric_limits<uint64_t>::max ();

    [[nodiscard]] inline bool IsCorpse () const
    {
        return removeAfterNs != std::numeric_limits<uint64_t>::max ();
    }

    struct Reflection final
    {
        Emergence::StandardLayout::FieldId objectId;
        Emergence::StandardLayout::FieldId health;
        Emergence::StandardLayout::FieldId maxHealth;
        Emergence::StandardLayout::FieldId removeAfterNs;
        Emergence::StandardLayout::Mapping mapping;
    };

    static const Reflection &Reflect () noexcept;
};

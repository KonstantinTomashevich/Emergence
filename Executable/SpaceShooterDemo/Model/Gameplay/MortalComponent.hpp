#pragma once

#include <Celerity/Standard/UniqueId.hpp>

#include <StandardLayout/Mapping.hpp>

struct MortalComponent final
{
    Emergence::Celerity::UniqueId objectId = Emergence::Celerity::INVALID_UNIQUE_ID;

    /// \brief If true, mortal will not receive damage.
    bool invincible = false;

    float health = 1.0f;

    float maxHealth = 1.0f;

    /// \brief If higher than zero, mortal will die after it lives for given amount of seconds.
    float maximumLifetimeS = 0.0f;

    uint64_t dieAfterNs = std::numeric_limits<uint64_t>::max ();

    float corpseLifetimeS = 2.0f;

    /// \details Timer for transform removal after death.
    uint64_t removeAfterNs = std::numeric_limits<uint64_t>::max ();

    [[nodiscard]] inline bool IsCorpse () const
    {
        return removeAfterNs != std::numeric_limits<uint64_t>::max ();
    }

    struct Reflection final
    {
        Emergence::StandardLayout::FieldId objectId;
        Emergence::StandardLayout::FieldId invincible;
        Emergence::StandardLayout::FieldId health;
        Emergence::StandardLayout::FieldId maxHealth;
        Emergence::StandardLayout::FieldId maximumLifetimeS;
        Emergence::StandardLayout::FieldId dieAfterNs;
        Emergence::StandardLayout::FieldId corpseLifetimeS;
        Emergence::StandardLayout::FieldId removeAfterNs;
        Emergence::StandardLayout::Mapping mapping;
    };

    static const Reflection &Reflect () noexcept;
};

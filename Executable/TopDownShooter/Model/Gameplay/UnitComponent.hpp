#pragma once

#include <Celerity/Standard/UniqueId.hpp>

#include <StandardLayout/Mapping.hpp>

struct UnitComponent final
{
    Emergence::Celerity::UniqueId objectId = Emergence::Celerity::INVALID_UNIQUE_ID;

    Emergence::Memory::UniqueString type;

    float health = 1.0f;

    float maxHealth = 1.0f;

    /// \details Timer for transform removal after death.
    uint64_t removeAfterNs = std::numeric_limits<uint64_t>::max ();

    bool canBeControlledByPlayer = false;

    bool controlledByPlayer = false;

    [[nodiscard]] inline bool IsCorpse () const
    {
        return removeAfterNs != std::numeric_limits<uint64_t>::max ();
    }

    struct Reflection final
    {
        Emergence::StandardLayout::FieldId objectId;
        Emergence::StandardLayout::FieldId type;
        Emergence::StandardLayout::FieldId health;
        Emergence::StandardLayout::FieldId maxHealth;
        Emergence::StandardLayout::FieldId removeAfterNs;
        Emergence::StandardLayout::FieldId canBeControlledByPlayer;
        Emergence::StandardLayout::FieldId controlledByPlayer;
        Emergence::StandardLayout::Mapping mapping;
    };

    static const Reflection &Reflect () noexcept;
};

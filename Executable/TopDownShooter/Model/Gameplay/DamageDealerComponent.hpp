#pragma once

#include <Celerity/Standard/UniqueId.hpp>

#include <StandardLayout/Mapping.hpp>

struct DamageDealerComponent final
{
    Emergence::Celerity::UniqueId objectId = Emergence::Celerity::INVALID_UNIQUE_ID;

    float damage = 0.0f;

    bool multiUse = false;

    bool enabled = true;

    struct Reflection final
    {
        Emergence::StandardLayout::FieldId objectId;
        Emergence::StandardLayout::FieldId damage;
        Emergence::StandardLayout::FieldId multiUse;
        Emergence::StandardLayout::FieldId enabled;
        Emergence::StandardLayout::Mapping mapping;
    };

    static Reflection &Reflect () noexcept;
};

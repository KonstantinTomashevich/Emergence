#pragma once

#include <StandardLayout/Mapping.hpp>

namespace Emergence::Celerity::Test
{
struct UnitConfig final
{
    Memory::UniqueString id;
    float health = 100.0f;
    float armor = 0.0f;
    float damagePerHit = 10.0f;
    float hitCastTime = 1.0f;

    bool operator== (const UnitConfig &_other) const = default;

    struct Reflection final
    {
        StandardLayout::FieldId id;
        StandardLayout::FieldId health;
        StandardLayout::FieldId armor;
        StandardLayout::FieldId damagePerHit;
        StandardLayout::FieldId hitCastTime;
        StandardLayout::Mapping mapping;
    };

    static const Reflection &Reflect () noexcept;
};

struct BuildingConfig final
{
    Memory::UniqueString id;
    float health = 1000.0f;
    float cost = 100.0f;
    float buildTime = 60.0f;

    bool operator== (const BuildingConfig &_other) const = default;

    struct Reflection final
    {
        StandardLayout::FieldId id;
        StandardLayout::FieldId health;
        StandardLayout::FieldId cost;
        StandardLayout::FieldId buildTime;
        StandardLayout::Mapping mapping;
    };

    static const Reflection &Reflect () noexcept;
};
} // namespace Emergence::Celerity::Test

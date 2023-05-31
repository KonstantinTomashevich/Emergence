#pragma once

#include <Celerity/Standard/UniqueId.hpp>

#include <StandardLayout/Mapping.hpp>

struct ShooterComponent final
{
    Emergence::Celerity::UniqueId objectId = Emergence::Celerity::INVALID_UNIQUE_ID;

    Emergence::Celerity::UniqueId shootingPointObjectId = Emergence::Celerity::INVALID_UNIQUE_ID;

    Emergence::Memory::UniqueString bulletPrototype;

    std::uint64_t coolDownNs = 0u;

    std::uint64_t coolingDownUntilNs = 0u;

    struct Reflection final
    {
        Emergence::StandardLayout::FieldId objectId;
        Emergence::StandardLayout::FieldId shootingPointObjectId;
        Emergence::StandardLayout::FieldId bulletPrototype;
        Emergence::StandardLayout::FieldId coolDownNs;
        Emergence::StandardLayout::FieldId coolingDownUntilNs;
        Emergence::StandardLayout::Mapping mapping;
    };

    static const Reflection &Reflect () noexcept;
};

#pragma once

#include <Celerity/Standard/UniqueId.hpp>

#include <StandardLayout/Mapping.hpp>

struct TeamComponent final
{
    Emergence::Celerity::UniqueId objectId = Emergence::Celerity::INVALID_UNIQUE_ID;

    std::uint8_t teamId = 0u;

    struct Reflection final
    {
        Emergence::StandardLayout::FieldId objectId;
        Emergence::StandardLayout::FieldId teamId;
        Emergence::StandardLayout::Mapping mapping;
    };

    static const Reflection &Reflect () noexcept;
};

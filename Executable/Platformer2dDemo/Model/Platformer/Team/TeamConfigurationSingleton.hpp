#pragma once

#include <StandardLayout/Mapping.hpp>

struct TeamConfigurationSingleton final
{
    uint8_t playerTeamId = 0u;

    struct Reflection final
    {
        Emergence::StandardLayout::FieldId playerTeamId;
        Emergence::StandardLayout::Mapping mapping;
    };

    static const Reflection &Reflect () noexcept;
};

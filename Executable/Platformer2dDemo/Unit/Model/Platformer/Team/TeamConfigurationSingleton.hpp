#pragma once

#include <Platformer2dDemoModelApi.hpp>

#include <StandardLayout/Mapping.hpp>

struct Platformer2dDemoModelApi TeamConfigurationSingleton final
{
    std::uint8_t playerTeamId = 0u;

    struct Platformer2dDemoModelApi Reflection final
    {
        Emergence::StandardLayout::FieldId playerTeamId;
        Emergence::StandardLayout::Mapping mapping;
    };

    static const Reflection &Reflect () noexcept;
};

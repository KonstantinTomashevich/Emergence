#pragma once

#include <StandardLayout/Mapping.hpp>

struct MortalitySettingsSingleton final
{
    uint64_t corpseLifetimeNs = 3000000000u; // 3 seconds.

    struct Reflection
    {
        Emergence::StandardLayout::FieldId corpseLifetimeNs;
        Emergence::StandardLayout::Mapping mapping;
    };

    static const Reflection &Reflect () noexcept;
};

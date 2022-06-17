#pragma once

#include <StandardLayout/Mapping.hpp>

struct MortalitySettingsSingleton final
{
    uint64_t corpseLifetimeNs = 1000000000u; // 1 second.

    struct Reflection
    {
        Emergence::StandardLayout::FieldId corpseLifetimeNs;
        Emergence::StandardLayout::Mapping mapping;
    };

    static const Reflection &Reflect () noexcept;
};

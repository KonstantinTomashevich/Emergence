#pragma once

#include <StandardLayout/Mapping.hpp>

struct InputConfigurationSingleton final
{
    uint8_t readyForRouting = 0u;
    std::uint64_t listenerObjectId = 0u;

    struct Reflection final
    {
        Emergence::StandardLayout::FieldId readyForRouting;
        Emergence::StandardLayout::FieldId listenerObjectId;
        Emergence::StandardLayout::Mapping mapping;
    };

    static const Reflection &Reflect () noexcept;
};

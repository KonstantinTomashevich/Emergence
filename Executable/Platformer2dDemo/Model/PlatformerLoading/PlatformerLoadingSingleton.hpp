#pragma once

#include <StandardLayout/Mapping.hpp>

struct PlatformerLoadingSingleton final
{
    bool assetsLoaded = false;

    bool dynamicsMaterialsLoaded = false;
    bool dynamicsMaterialsLoadingRequested = false;

    bool movementConfigurationsLoaded = false;
    bool movementConfigurationsLoadingRequested = false;

    uint64_t loadingStartTimeNs = 0u;

    struct Reflection final
    {
        Emergence::StandardLayout::FieldId assetsLoaded;
        Emergence::StandardLayout::FieldId dynamicsMaterialsLoaded;
        Emergence::StandardLayout::FieldId dynamicsMaterialsLoadingRequested;
        Emergence::StandardLayout::FieldId movementConfigurationsLoaded;
        Emergence::StandardLayout::FieldId movementConfigurationsLoadingRequested;
        Emergence::StandardLayout::FieldId loadingStartTimeNs;
        Emergence::StandardLayout::Mapping mapping;
    };

    static const Reflection &Reflect () noexcept;
};

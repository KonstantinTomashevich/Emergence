#pragma once

#include <Platformer2dDemoModelApi.hpp>

#include <StandardLayout/Mapping.hpp>

struct Platformer2dDemoModelApi PlatformerLoadingSingleton final
{
    bool assetsLoaded = false;

    bool characterAnimationConfigurationsLoaded = false;
    bool characterAnimationConfigurationsLoadingRequested = false;

    bool dynamicsMaterialsLoaded = false;
    bool dynamicsMaterialsLoadingRequested = false;

    bool movementConfigurationsLoaded = false;
    bool movementConfigurationsLoadingRequested = false;

    std::uint64_t loadingStartTimeNs = 0u;

    struct Platformer2dDemoModelApi Reflection final
    {
        Emergence::StandardLayout::FieldId assetsLoaded;
        Emergence::StandardLayout::FieldId characterAnimationConfigurationsLoaded;
        Emergence::StandardLayout::FieldId characterAnimationConfigurationsLoadingRequested;
        Emergence::StandardLayout::FieldId dynamicsMaterialsLoaded;
        Emergence::StandardLayout::FieldId dynamicsMaterialsLoadingRequested;
        Emergence::StandardLayout::FieldId movementConfigurationsLoaded;
        Emergence::StandardLayout::FieldId movementConfigurationsLoadingRequested;
        Emergence::StandardLayout::FieldId loadingStartTimeNs;
        Emergence::StandardLayout::Mapping mapping;
    };

    static const Reflection &Reflect () noexcept;
};

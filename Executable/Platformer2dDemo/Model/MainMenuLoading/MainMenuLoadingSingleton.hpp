#pragma once

#include <StandardLayout/Mapping.hpp>

struct MainMenuLoadingSingleton final
{
    bool levelsConfigurationLoaded = false;
    bool mainMenuInitialized = false;
    bool assetsLoaded = false;

    struct Reflection final
    {
        Emergence::StandardLayout::FieldId levelsConfigurationLoaded;
        Emergence::StandardLayout::FieldId mainMenuInitialized;
        Emergence::StandardLayout::FieldId assetsLoaded;
        Emergence::StandardLayout::Mapping mapping;
    };

    static const Reflection &Reflect () noexcept;
};
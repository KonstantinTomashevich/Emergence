#pragma once

#include <StandardLayout/Mapping.hpp>

struct MainMenuLoadingSingleton final
{
    bool assetsLoaded = false;

    struct Reflection final
    {
        Emergence::StandardLayout::FieldId assetsLoaded;
        Emergence::StandardLayout::Mapping mapping;
    };

    static const Reflection &Reflect () noexcept;
};

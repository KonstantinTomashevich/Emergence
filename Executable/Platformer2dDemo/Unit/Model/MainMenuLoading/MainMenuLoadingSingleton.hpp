#pragma once

#include <Platformer2dDemoModelApi.hpp>

#include <StandardLayout/Mapping.hpp>

struct Platformer2dDemoModelApi MainMenuLoadingSingleton final
{
    bool assetsLoaded = false;

    struct Platformer2dDemoModelApi Reflection final
    {
        Emergence::StandardLayout::FieldId assetsLoaded;
        Emergence::StandardLayout::Mapping mapping;
    };

    static const Reflection &Reflect () noexcept;
};

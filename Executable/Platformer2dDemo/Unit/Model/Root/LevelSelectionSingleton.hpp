#pragma once

#include <Platformer2dDemoModelApi.hpp>

#include <StandardLayout/Mapping.hpp>

struct Platformer2dDemoModelApi LevelSelectionSingleton final
{
    Emergence::Memory::UniqueString selectedLevelName;

    struct Platformer2dDemoModelApi Reflection final
    {
        Emergence::StandardLayout::FieldId selectedLevelName;
        Emergence::StandardLayout::Mapping mapping;
    };

    static const Reflection &Reflect () noexcept;
};

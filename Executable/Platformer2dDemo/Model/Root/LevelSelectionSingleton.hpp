#pragma once

#include <StandardLayout/Mapping.hpp>

struct LevelSelectionSingleton final
{
    Emergence::Memory::UniqueString selectedLevelName;

    struct Reflection final
    {
        Emergence::StandardLayout::FieldId selectedLevelName;
        Emergence::StandardLayout::Mapping mapping;
    };

    static const Reflection &Reflect () noexcept;
};

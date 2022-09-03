#pragma once

#include <StandardLayout/Mapping.hpp>

struct DynamicsMaterialLoadingSingleton final
{
    bool finished = false;

    struct Reflection
    {
        Emergence::StandardLayout::FieldId finished;
        Emergence::StandardLayout::Mapping mapping;
    };

    static const Reflection &Reflect () noexcept;
};

#pragma once

#include <Input/InputMapping.hpp>

struct NormalInputMappingSingleton final
{
    InputMapping inputMapping;

    struct Reflection final
    {
        Emergence::StandardLayout::FieldId inputMapping;
        Emergence::StandardLayout::Mapping mapping;
    };

    static const Reflection &Reflect () noexcept;
};

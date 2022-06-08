#pragma once

#include <StandardLayout/Mapping.hpp>

struct Color final
{
    float r = 1.0f;
    float g = 1.0f;
    float b = 1.0f;
    float a = 1.0f;

    struct Reflection final
    {
        Emergence::StandardLayout::FieldId r;
        Emergence::StandardLayout::FieldId g;
        Emergence::StandardLayout::FieldId b;
        Emergence::StandardLayout::FieldId a;
        Emergence::StandardLayout::Mapping mapping;
    };

    static const Reflection &Reflect () noexcept;
};

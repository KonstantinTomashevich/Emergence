#pragma once

#include <StandardLayout/Mapping.hpp>

struct AssemblyDescriptorLoadingSingleton final
{
    bool started = false;
    bool finished = false;

    struct Reflection
    {
        Emergence::StandardLayout::FieldId started;
        Emergence::StandardLayout::FieldId finished;
        Emergence::StandardLayout::Mapping mapping;
    };

    static const Reflection &Reflect () noexcept;
};

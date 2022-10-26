#pragma once

#include <StandardLayout/Mapping.hpp>

struct Settings
{
    std::uint32_t width = 1920u;
    std::uint32_t height = 1080u;
    bool fullscreen = false;
    bool vsync = false;

    struct Reflection final
    {
        Emergence::StandardLayout::FieldId width;
        Emergence::StandardLayout::FieldId height;
        Emergence::StandardLayout::FieldId fullscreen;
        Emergence::StandardLayout::FieldId vsync;
        Emergence::StandardLayout::Mapping mapping;
    };

    static const Reflection &Reflect () noexcept;
};

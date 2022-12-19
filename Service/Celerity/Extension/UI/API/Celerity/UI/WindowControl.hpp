#pragma once

#include <Celerity/Standard/UniqueId.hpp>

#include <Math/Vector2f.hpp>

#include <StandardLayout/Mapping.hpp>

namespace Emergence::Celerity
{
struct WindowControl final
{
    UniqueId nodeId = INVALID_UNIQUE_ID;
    Memory::UniqueString viewportName;
    Container::Utf8String title;

    bool closable = true;
    bool minimizable = true;
    bool resizable = true;
    bool movable = true;
    bool hasTitleBar = true;
    bool pack = false;

    Math::Vector2f anchor;
    int32_t x = 0;
    int32_t y = 0;
    uint32_t width = 0u;
    uint32_t height = 0u;

    struct Reflection final
    {
        StandardLayout::FieldId nodeId;
        StandardLayout::FieldId viewportName;

        StandardLayout::FieldId closable;
        StandardLayout::FieldId minimizable;
        StandardLayout::FieldId resizable;
        StandardLayout::FieldId movable;
        StandardLayout::FieldId hasTitleBar;
        StandardLayout::FieldId pack;

        StandardLayout::FieldId anchor;
        StandardLayout::FieldId x;
        StandardLayout::FieldId y;
        StandardLayout::FieldId width;
        StandardLayout::FieldId height;
        StandardLayout::Mapping mapping;
    };

    static const Reflection &Reflect () noexcept;
};
} // namespace Emergence::Celerity

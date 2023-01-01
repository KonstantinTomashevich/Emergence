#pragma once

#include <Celerity/Standard/UniqueId.hpp>

#include <StandardLayout/Mapping.hpp>

namespace Emergence::Celerity
{
enum class UIStyleColorPropertyName
{
    TEXT = 0u,
    DISABLED_TEXT,
    WINDOW_BACKGROUND,
    CONTAINER_BACKGROUND,
    POPUP_BACKGROUND,
    BORDER,
    BORDER_SHADOW,
    TITLE_BACKGROUND,
    TITLE_BACKGROUND_ACTIVE,
    TITLE_BACKGROUND_COLLAPSED,
    MENUBAR_BACKGROUND,
    SCROLLBAR_BACKGROUND,
    SCROLLBAR_GRAB,
    SCROLLBAR_GRAB_HOVERED,
    SCROLLBAR_GRAB_ACTIVE,
    CHECK_MARK,
    SLIDER_GRAB,
    BUTTON,
    BUTTON_HOVERED,
    BUTTON_ACTIVE,
    HEADER,
    HEADER_HOVERED,
    HEADER_ACTIVE,
    SEPARATOR,
    SEPARATOR_HOVERED,
    SEPARATOR_ACTIVE,
    RESIZE_GRIP,
    RESIZE_GRIP_HOVERED,
    RESIZE_GRIP_ACTIVE,
    TAB,
    TAB_HOVERED,
    TAB_ACTIVE,
    TAB_UNFOCUSED,
    TAB_UNFOCUSED_ACTIVE,
};

struct UIStyleColorProperty final
{
    Memory::UniqueString styleId;
    UIStyleColorPropertyName property = UIStyleColorPropertyName::TEXT;

    union
    {
        struct
        {
            float red = 0.0f;
            float green = 0.0f;
            float blue = 0.0f;
            float alpha = 1.0f;
        };

        float rgba[4u];
    };

    struct Reflection final
    {
        StandardLayout::FieldId styleId;
        StandardLayout::FieldId property;
        StandardLayout::FieldId red;
        StandardLayout::FieldId green;
        StandardLayout::FieldId blue;
        StandardLayout::FieldId alpha;
        StandardLayout::Mapping mapping;
    };

    static const Reflection &Reflect () noexcept;
};

enum class UIStyleFloatPropertyName
{
    ALPHA,
    DISABLED_ALPHA,
    WINDOW_ROUNDING,
    WINDOW_BORDER_SIZE,
    CONTAINER_ROUNDING,
    CONTAINER_BORDER_SIZE,
    POPUP_ROUNDING,
    POPUP_BORDER_SIZE,
    INDENT_SPACING,
    SCROLLBAR_SIZE,
    SCROLLBAR_ROUNDING,
    GRAB_MIN_SIZE,
    GRAB_ROUNDING,
    TAB_ROUNDING,
};

struct UIStyleFloatProperty final
{
    Memory::UniqueString styleId;
    UIStyleFloatPropertyName property = UIStyleFloatPropertyName::ALPHA;
    float value = 0.0f;

    struct Reflection final
    {
        StandardLayout::FieldId styleId;
        StandardLayout::FieldId property;
        StandardLayout::FieldId value;
        StandardLayout::Mapping mapping;
    };

    static const Reflection &Reflect () noexcept;
};

enum class UIStyleFloatPairPropertyName
{
    WINDOW_PADDING,
    WINDOW_MIN_SIZE,
    WINDOW_TITLE_ALIGN,
    ITEM_SPACING,
    ITEM_INNER_SPACING,
    BUTTON_TEXT_ALIGN,
    SELECTABLE_TEXT_ALIGN,
};

struct UIStyleFloatPairProperty final
{
    Memory::UniqueString styleId;
    UIStyleFloatPairPropertyName property = UIStyleFloatPairPropertyName::WINDOW_PADDING;
    float x = 0.0f;
    float y = 0.0f;

    struct Reflection final
    {
        StandardLayout::FieldId styleId;
        StandardLayout::FieldId property;
        StandardLayout::FieldId x;
        StandardLayout::FieldId y;
        StandardLayout::Mapping mapping;
    };

    static const Reflection &Reflect () noexcept;
};

struct UIStyleFontProperty final
{
    Memory::UniqueString styleId;
    UniqueId assetUserId = INVALID_UNIQUE_ID;
    Memory::UniqueString fontId;

    struct Reflection final
    {
        StandardLayout::FieldId styleId;
        StandardLayout::FieldId assetUserId;
        StandardLayout::FieldId fontId;
        StandardLayout::Mapping mapping;
    };

    static const Reflection &Reflect () noexcept;
};
} // namespace Emergence::Celerity

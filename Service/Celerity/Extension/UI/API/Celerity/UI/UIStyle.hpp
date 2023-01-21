#pragma once

#include <Celerity/Standard/UniqueId.hpp>

#include <StandardLayout/Mapping.hpp>

namespace Emergence::Celerity
{
/// \brief Enumerates supported style properties that have color type.
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
    BUTTON,
    BUTTON_HOVERED,
    BUTTON_ACTIVE,
    HEADER,
    HEADER_HOVERED,
    HEADER_ACTIVE,
    RESIZE_GRIP,
    RESIZE_GRIP_HOVERED,
    RESIZE_GRIP_ACTIVE,
};

/// \brief Attaches property of color type to a style.
struct UIStyleColorProperty final
{
    /// \brief Unique identifier of style to which this property instance belongs.
    Memory::UniqueString styleId;

    /// \brief Target property name.
    UIStyleColorPropertyName property = UIStyleColorPropertyName::TEXT;

    union
    {
        struct
        {
            float red;
            float green;
            float blue;
            float alpha;
        };

        /// \brief Value that is assigned to property in RGBA format.
        float rgba[4u] {0.0f, 0.0f, 0.0f, 0.0f};
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

/// \brief Enumerates supported style properties that have float type.
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
};

/// \brief Attaches property of float type to a style.
struct UIStyleFloatProperty final
{
    /// \brief Unique identifier of style to which this property instance belongs.
    Memory::UniqueString styleId;

    /// \brief Target property name.
    UIStyleFloatPropertyName property = UIStyleFloatPropertyName::ALPHA;

    /// \brief Value that is assigned to property.
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

/// \brief Enumerates supported style properties that have float pair type.
enum class UIStyleFloatPairPropertyName
{
    WINDOW_PADDING,
    WINDOW_MIN_SIZE,
    WINDOW_TITLE_ALIGN,
    ITEM_SPACING,
    ITEM_INNER_SPACING,
    BUTTON_TEXT_ALIGN,
};

/// \brief Attaches property of float pair type to a style.
struct UIStyleFloatPairProperty final
{
    /// \brief Unique identifier of style to which this property instance belongs.
    Memory::UniqueString styleId;

    /// \brief Target property name.
    UIStyleFloatPairPropertyName property = UIStyleFloatPairPropertyName::WINDOW_PADDING;

    /// \brief First component of the value that is assigned to property.
    float x = 0.0f;

    /// \brief Second component of the value that is assigned to property.
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

/// \brief Defines font that is attached to given style.
/// \invariant Only one font attachment per style is allowed.
struct UIStyleFontProperty final
{
    /// \brief Unique identifier of style to which this property instance belongs.
    Memory::UniqueString styleId;

    /// \brief Asset user id is required, because we reference font asset.
    UniqueId assetUserId = INVALID_UNIQUE_ID;

    /// \brief Id of a font asset to be used.
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

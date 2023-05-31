#include <Celerity/UI/UIStyle.hpp>

#include <StandardLayout/MappingRegistration.hpp>

namespace Emergence::Celerity
{
UIStyleColorProperty::UIStyleColorProperty () noexcept
    : red (0.0f),
      green (0.0f),
      blue (0.0f),
      alpha (0.0f)
{
}

const UIStyleColorProperty::Reflection &UIStyleColorProperty::Reflect () noexcept
{
    static const Reflection reflection = [] ()
    {
        EMERGENCE_MAPPING_REGISTRATION_BEGIN (UIStyleColorProperty);
        EMERGENCE_MAPPING_REGISTER_REGULAR (styleId);
        EMERGENCE_MAPPING_REGISTER_REGULAR (property);
        EMERGENCE_MAPPING_REGISTER_REGULAR (red);
        EMERGENCE_MAPPING_REGISTER_REGULAR (green);
        EMERGENCE_MAPPING_REGISTER_REGULAR (blue);
        EMERGENCE_MAPPING_REGISTER_REGULAR (alpha);
        EMERGENCE_MAPPING_REGISTRATION_END ();
    }();

    return reflection;
}

const UIStyleFloatProperty::Reflection &UIStyleFloatProperty::Reflect () noexcept
{
    static const Reflection reflection = [] ()
    {
        EMERGENCE_MAPPING_REGISTRATION_BEGIN (UIStyleFloatProperty);
        EMERGENCE_MAPPING_REGISTER_REGULAR (styleId);
        EMERGENCE_MAPPING_REGISTER_REGULAR (property);
        EMERGENCE_MAPPING_REGISTER_REGULAR (value);
        EMERGENCE_MAPPING_REGISTRATION_END ();
    }();

    return reflection;
}

const UIStyleFloatPairProperty::Reflection &UIStyleFloatPairProperty::Reflect () noexcept
{
    static const Reflection reflection = [] ()
    {
        EMERGENCE_MAPPING_REGISTRATION_BEGIN (UIStyleFloatPairProperty);
        EMERGENCE_MAPPING_REGISTER_REGULAR (styleId);
        EMERGENCE_MAPPING_REGISTER_REGULAR (property);
        EMERGENCE_MAPPING_REGISTER_REGULAR (x);
        EMERGENCE_MAPPING_REGISTER_REGULAR (y);
        EMERGENCE_MAPPING_REGISTRATION_END ();
    }();

    return reflection;
}

const UIStyleFontProperty::Reflection &UIStyleFontProperty::Reflect () noexcept
{
    static const Reflection reflection = [] ()
    {
        EMERGENCE_MAPPING_REGISTRATION_BEGIN (UIStyleFontProperty);
        EMERGENCE_MAPPING_REGISTER_REGULAR (styleId);
        EMERGENCE_MAPPING_REGISTER_REGULAR (fontId);
        EMERGENCE_MAPPING_REGISTRATION_END ();
    }();

    return reflection;
}
} // namespace Emergence::Celerity

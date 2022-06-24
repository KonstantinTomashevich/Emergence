#include <Render/Color.hpp>

#include <StandardLayout/MappingRegistration.hpp>

const Color::Reflection &Color::Reflect () noexcept
{
    static Reflection reflection = [] ()
    {
        EMERGENCE_MAPPING_REGISTRATION_BEGIN (Color)
        EMERGENCE_MAPPING_REGISTER_REGULAR (r)
        EMERGENCE_MAPPING_REGISTER_REGULAR (g)
        EMERGENCE_MAPPING_REGISTER_REGULAR (b)
        EMERGENCE_MAPPING_REGISTER_REGULAR (a)
        EMERGENCE_MAPPING_REGISTRATION_END ()
    }();

    return reflection;
}

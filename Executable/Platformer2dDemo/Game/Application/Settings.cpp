#include <Application/Settings.hpp>

#include <StandardLayout/MappingRegistration.hpp>

const Settings::Reflection &Settings::Reflect () noexcept
{
    static Reflection reflection = [] ()
    {
        EMERGENCE_MAPPING_REGISTRATION_BEGIN (Settings);
        EMERGENCE_MAPPING_REGISTER_REGULAR (width);
        EMERGENCE_MAPPING_REGISTER_REGULAR (height);
        EMERGENCE_MAPPING_REGISTER_REGULAR (fullscreen);
        EMERGENCE_MAPPING_REGISTER_REGULAR (vsync);
        EMERGENCE_MAPPING_REGISTRATION_END ();
    }();

    return reflection;
}

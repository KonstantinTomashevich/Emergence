#include <Platformer/PlatformerSingleton.hpp>

#include <StandardLayout/MappingRegistration.hpp>

const PlatformerSingleton::Reflection &PlatformerSingleton::Reflect () noexcept
{
    static const Reflection reflection = [] ()
    {
        EMERGENCE_MAPPING_REGISTRATION_BEGIN (PlatformerSingleton);
        EMERGENCE_MAPPING_REGISTER_REGULAR (state);
        EMERGENCE_MAPPING_REGISTRATION_END ();
    }();

    return reflection;
}

#include <Platformer/Movement/MovementConfigurationSingleton.hpp>

#include <StandardLayout/MappingRegistration.hpp>

const MovementConfigurationSingleton::Reflection &MovementConfigurationSingleton::Reflect () noexcept
{
    static const Reflection reflection = [] ()
    {
        EMERGENCE_MAPPING_REGISTRATION_BEGIN (MovementConfigurationSingleton);
        EMERGENCE_MAPPING_REGISTER_REGULAR (jumpActuationThreshold);
        EMERGENCE_MAPPING_REGISTER_REGULAR (crouchOrSlideActuationThreshold);
        EMERGENCE_MAPPING_REGISTER_REGULAR (runActuationThreshold);
        EMERGENCE_MAPPING_REGISTRATION_END ();
    }();

    return reflection;
}

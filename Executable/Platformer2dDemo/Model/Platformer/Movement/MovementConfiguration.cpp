#include <Platformer/Movement/MovementConfiguration.hpp>

#include <StandardLayout/MappingRegistration.hpp>

const MovementConfiguration::Reflection &MovementConfiguration::Reflect () noexcept
{
    static const Reflection reflection = [] ()
    {
        EMERGENCE_MAPPING_REGISTRATION_BEGIN (MovementConfiguration);
        EMERGENCE_MAPPING_REGISTER_REGULAR (id);
        EMERGENCE_MAPPING_REGISTER_REGULAR (runVelocity);
        EMERGENCE_MAPPING_REGISTER_REGULAR (jumpVelocity);
        EMERGENCE_MAPPING_REGISTER_REGULAR (groundMaxSlopeDeg);
        EMERGENCE_MAPPING_REGISTER_REGULAR (airControlVelocity);
        EMERGENCE_MAPPING_REGISTER_REGULAR (rollVelocity);
        EMERGENCE_MAPPING_REGISTER_REGULAR (rollDurationS);
        EMERGENCE_MAPPING_REGISTER_REGULAR (slideVelocity);
        EMERGENCE_MAPPING_REGISTER_REGULAR (slideDurationS);
        EMERGENCE_MAPPING_REGISTER_REGULAR (allowJump);
        EMERGENCE_MAPPING_REGISTER_REGULAR (allowCrouch);
        EMERGENCE_MAPPING_REGISTER_REGULAR (allowRoll);
        EMERGENCE_MAPPING_REGISTER_REGULAR (allowSlide);
        EMERGENCE_MAPPING_REGISTRATION_END ();
    }();

    return reflection;
}

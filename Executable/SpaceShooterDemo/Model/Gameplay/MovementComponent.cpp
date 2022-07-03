#include <Gameplay/MovementComponent.hpp>

#include <StandardLayout/MappingRegistration.hpp>

const MovementComponent::Reflection &MovementComponent::Reflect () noexcept
{
    static Reflection reflection = [] ()
    {
        EMERGENCE_MAPPING_REGISTRATION_BEGIN (MovementComponent);
        EMERGENCE_MAPPING_REGISTER_REGULAR (objectId);
        EMERGENCE_MAPPING_REGISTER_REGULAR (maxLinearSpeed);
        EMERGENCE_MAPPING_REGISTER_REGULAR (linearAcceleration);
        EMERGENCE_MAPPING_REGISTER_REGULAR (maxAngularSpeed);
        EMERGENCE_MAPPING_REGISTER_REGULAR (angularAcceleration);
        EMERGENCE_MAPPING_REGISTER_REGULAR (linearVelocityMask);
        EMERGENCE_MAPPING_REGISTER_REGULAR (angularVelocityMask);
        EMERGENCE_MAPPING_REGISTRATION_END ();
    }();

    return reflection;
}

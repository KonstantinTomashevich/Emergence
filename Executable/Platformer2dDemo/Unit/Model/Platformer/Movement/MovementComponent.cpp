#include <Platformer/Movement/MovementComponent.hpp>

#include <StandardLayout/MappingRegistration.hpp>

const MovementComponent::Reflection &MovementComponent::Reflect () noexcept
{
    static const Reflection reflection = [] ()
    {
        EMERGENCE_MAPPING_REGISTRATION_BEGIN (MovementComponent);
        EMERGENCE_MAPPING_REGISTER_REGULAR (objectId);
        EMERGENCE_MAPPING_REGISTER_REGULAR (configurationId);
        EMERGENCE_MAPPING_REGISTER_REGULAR (state);
        EMERGENCE_MAPPING_REGISTER_REGULAR (stateStartTimeNs);
        EMERGENCE_MAPPING_REGISTER_REGULAR (lastMovementVelocity);
        EMERGENCE_MAPPING_REGISTER_REGULAR (framesInAir);
        EMERGENCE_MAPPING_REGISTER_REGULAR (framesOnGround);
        EMERGENCE_MAPPING_REGISTRATION_END ();
    }();

    return reflection;
}

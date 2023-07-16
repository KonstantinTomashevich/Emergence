#include <Platformer/Movement/MovementBlockedComponent.hpp>

#include <StandardLayout/MappingRegistration.hpp>

const MovementBlockedComponent::Reflection &MovementBlockedComponent::Reflect () noexcept
{
    static const Reflection reflection = [] ()
    {
        EMERGENCE_MAPPING_REGISTRATION_BEGIN (MovementBlockedComponent);
        EMERGENCE_MAPPING_REGISTER_REGULAR (objectId);
        EMERGENCE_MAPPING_REGISTER_REGULAR (reason);
        EMERGENCE_MAPPING_REGISTRATION_END ();
    }();

    return reflection;
}

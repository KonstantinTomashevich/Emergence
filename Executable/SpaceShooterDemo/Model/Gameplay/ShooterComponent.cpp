#include <Gameplay/ShooterComponent.hpp>

#include <StandardLayout/MappingRegistration.hpp>

ShooterComponent::Reflection &ShooterComponent::Reflect () noexcept
{
    static Reflection reflection = [] ()
    {
        EMERGENCE_MAPPING_REGISTRATION_BEGIN (ShooterComponent);
        EMERGENCE_MAPPING_REGISTER_REGULAR (objectId);
        EMERGENCE_MAPPING_REGISTER_REGULAR (shootingPointObjectId);
        EMERGENCE_MAPPING_REGISTER_REGULAR (bulletPrototype);
        EMERGENCE_MAPPING_REGISTER_REGULAR (coolDownNs);
        EMERGENCE_MAPPING_REGISTER_REGULAR (coolingDownUntilNs);
        EMERGENCE_MAPPING_REGISTRATION_END ();
    }();

    return reflection;
}

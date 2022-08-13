#include <Gameplay/MortalComponent.hpp>

#include <StandardLayout/MappingRegistration.hpp>

const MortalComponent::Reflection &MortalComponent::Reflect () noexcept
{
    static Reflection reflection = [] ()
    {
        EMERGENCE_MAPPING_REGISTRATION_BEGIN (MortalComponent);
        EMERGENCE_MAPPING_REGISTER_REGULAR (objectId);
        EMERGENCE_MAPPING_REGISTER_REGULAR (invincible);
        EMERGENCE_MAPPING_REGISTER_REGULAR (health);
        EMERGENCE_MAPPING_REGISTER_REGULAR (maxHealth);
        EMERGENCE_MAPPING_REGISTER_REGULAR (maximumLifetimeS);
        EMERGENCE_MAPPING_REGISTER_REGULAR (dieAfterNs);
        EMERGENCE_MAPPING_REGISTER_REGULAR (corpseLifetimeS);
        EMERGENCE_MAPPING_REGISTER_REGULAR (removeAfterNs);
        EMERGENCE_MAPPING_REGISTRATION_END ();
    }();

    return reflection;
}

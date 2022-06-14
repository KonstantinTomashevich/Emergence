#include <Gameplay/UnitComponent.hpp>

#include <StandardLayout/MappingRegistration.hpp>

const UnitComponent::Reflection &UnitComponent::Reflect () noexcept
{
    static Reflection reflection = [] ()
    {
        EMERGENCE_MAPPING_REGISTRATION_BEGIN (UnitComponent)
        EMERGENCE_MAPPING_REGISTER_REGULAR (objectId)
        EMERGENCE_MAPPING_REGISTER_UNIQUE_STRING (type)
        EMERGENCE_MAPPING_REGISTER_REGULAR (health)
        EMERGENCE_MAPPING_REGISTER_REGULAR (maxHealth)
        EMERGENCE_MAPPING_REGISTER_REGULAR (removeAfterNs)
        EMERGENCE_MAPPING_REGISTER_REGULAR (canBeControlledByPlayer)
        EMERGENCE_MAPPING_REGISTER_REGULAR (controlledByPlayer)
        EMERGENCE_MAPPING_REGISTRATION_END()
    }();

    return reflection;
}

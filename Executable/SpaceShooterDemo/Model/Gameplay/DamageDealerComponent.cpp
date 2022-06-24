#include <Gameplay/DamageDealerComponent.hpp>

#include <StandardLayout/MappingRegistration.hpp>

DamageDealerComponent::Reflection &DamageDealerComponent::Reflect () noexcept
{
    static Reflection reflection = [] ()
    {
        EMERGENCE_MAPPING_REGISTRATION_BEGIN (DamageDealerComponent)
        EMERGENCE_MAPPING_REGISTER_REGULAR (objectId)
        EMERGENCE_MAPPING_REGISTER_REGULAR (damage)
        EMERGENCE_MAPPING_REGISTER_REGULAR (multiUse)
        EMERGENCE_MAPPING_REGISTER_REGULAR (enabled)
        EMERGENCE_MAPPING_REGISTRATION_END ()
    }();

    return reflection;
}

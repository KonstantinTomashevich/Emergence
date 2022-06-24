#include <Gameplay/PrototypeComponent.hpp>

#include <StandardLayout/MappingRegistration.hpp>

PrototypeComponent::Reflection &PrototypeComponent::Reflect () noexcept
{
    static Reflection reflection = [] ()
    {
        EMERGENCE_MAPPING_REGISTRATION_BEGIN (PrototypeComponent)
        EMERGENCE_MAPPING_REGISTER_REGULAR (objectId)
        EMERGENCE_MAPPING_REGISTER_UNIQUE_STRING (prototype)
        EMERGENCE_MAPPING_REGISTRATION_END ()
    }();

    return reflection;
}

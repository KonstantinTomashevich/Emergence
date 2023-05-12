#include <Platformer/Control/ControllableComponent.hpp>

#include <StandardLayout/MappingRegistration.hpp>

const ControllableComponent::Reflection &ControllableComponent::Reflect () noexcept
{
    static const Reflection reflection = [] ()
    {
        EMERGENCE_MAPPING_REGISTRATION_BEGIN (ControllableComponent);
        EMERGENCE_MAPPING_REGISTER_REGULAR (objectId);
        EMERGENCE_MAPPING_REGISTER_REGULAR (control);
        EMERGENCE_MAPPING_REGISTRATION_END ();
    }();

    return reflection;
}

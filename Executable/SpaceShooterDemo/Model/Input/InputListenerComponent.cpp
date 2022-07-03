#include <Input/InputListenerComponent.hpp>

#include <StandardLayout/MappingRegistration.hpp>

const InputListenerComponent::Reflection &InputListenerComponent::Reflect () noexcept
{
    static Reflection reflection = [] ()
    {
        EMERGENCE_MAPPING_REGISTRATION_BEGIN (InputListenerComponent);
        EMERGENCE_MAPPING_REGISTER_REGULAR (objectId);
        EMERGENCE_MAPPING_REGISTER_REGULAR (actions);
        EMERGENCE_MAPPING_REGISTRATION_END ();
    }();

    return reflection;
}

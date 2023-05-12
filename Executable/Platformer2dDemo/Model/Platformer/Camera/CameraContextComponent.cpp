#include <Platformer/Camera/CameraContextComponent.hpp>

#include <StandardLayout/MappingRegistration.hpp>

const CameraContextComponent::Reflection &CameraContextComponent::Reflect () noexcept
{
    static const Reflection reflection = [] ()
    {
        EMERGENCE_MAPPING_REGISTRATION_BEGIN (CameraContextComponent);
        EMERGENCE_MAPPING_REGISTER_REGULAR (objectId);
        EMERGENCE_MAPPING_REGISTER_REGULAR (target);
        EMERGENCE_MAPPING_REGISTRATION_END ();
    }();

    return reflection;
}

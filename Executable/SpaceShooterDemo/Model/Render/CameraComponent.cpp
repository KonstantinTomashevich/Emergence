#include <Render/CameraComponent.hpp>

#include <StandardLayout/MappingRegistration.hpp>

CameraComponent::CameraComponent () noexcept = default;

CameraComponent::~CameraComponent () noexcept = default;

const CameraComponent::Reflection &CameraComponent::Reflect () noexcept
{
    static Reflection reflection = [] ()
    {
        EMERGENCE_MAPPING_REGISTRATION_BEGIN (CameraComponent);
        EMERGENCE_MAPPING_REGISTER_REGULAR (objectId);
        EMERGENCE_MAPPING_REGISTER_REGULAR (fieldOfViewRad);
        EMERGENCE_MAPPING_REGISTER_POINTER_AS_REGULAR (implementationHandle);
        EMERGENCE_MAPPING_REGISTRATION_END ();
    }();

    return reflection;
}

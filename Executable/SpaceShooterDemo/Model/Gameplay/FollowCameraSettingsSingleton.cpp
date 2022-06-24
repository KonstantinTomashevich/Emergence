#include <Gameplay/FollowCameraSettingsSingleton.hpp>

#include <StandardLayout/MappingRegistration.hpp>

const FollowCameraSettingsSingleton::Reflection &FollowCameraSettingsSingleton::Reflect () noexcept
{
    static Reflection reflection = [] ()
    {
        EMERGENCE_MAPPING_REGISTRATION_BEGIN (FollowCameraSettingsSingleton)
        EMERGENCE_MAPPING_REGISTER_NESTED_OBJECT (cameraLocalTransform)
        EMERGENCE_MAPPING_REGISTRATION_END ()
    }();

    return reflection;
}

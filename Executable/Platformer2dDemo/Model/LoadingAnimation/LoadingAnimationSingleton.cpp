#include <LoadingAnimation//LoadingAnimationSingleton.hpp>

#include <StandardLayout/MappingRegistration.hpp>

const LoadingAnimationSingleton::Reflection &LoadingAnimationSingleton::Reflect () noexcept
{
    static const Reflection reflection = [] ()
    {
        EMERGENCE_MAPPING_REGISTRATION_BEGIN (LoadingAnimationSingleton);
        EMERGENCE_MAPPING_REGISTER_REGULAR (required);
        EMERGENCE_MAPPING_REGISTER_REGULAR (instanced);
        EMERGENCE_MAPPING_REGISTER_REGULAR (cameraObjectId);
        EMERGENCE_MAPPING_REGISTER_REGULAR (spriteObjectId);
        EMERGENCE_MAPPING_REGISTRATION_END ();
    }();

    return reflection;
}

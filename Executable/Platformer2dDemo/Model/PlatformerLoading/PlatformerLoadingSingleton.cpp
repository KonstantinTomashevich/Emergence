#include <PlatformerLoading/PlatformerLoadingSingleton.hpp>

#include <StandardLayout/MappingRegistration.hpp>

const PlatformerLoadingSingleton::Reflection &PlatformerLoadingSingleton::Reflect () noexcept
{
    static const Reflection reflection = [] ()
    {
        EMERGENCE_MAPPING_REGISTRATION_BEGIN (PlatformerLoadingSingleton);
        EMERGENCE_MAPPING_REGISTER_REGULAR (assetsLoaded);
        EMERGENCE_MAPPING_REGISTER_REGULAR (loadingStartTimeNs);
        EMERGENCE_MAPPING_REGISTRATION_END ();
    }();

    return reflection;
}

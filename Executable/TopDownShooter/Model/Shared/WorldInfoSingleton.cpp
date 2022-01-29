#include <Shared/WorldInfoSingleton.hpp>

#include <StandardLayout/MappingRegistration.hpp>

const WorldInfoSingleton::Reflection &WorldInfoSingleton::Reflect () noexcept
{
    static Reflection reflection = [] ()
    {
        EMERGENCE_MAPPING_REGISTRATION_BEGIN (WorldInfoSingleton)
        EMERGENCE_MAPPING_REGISTER_REGULAR (fixedUpdateHappened)
        EMERGENCE_MAPPING_REGISTRATION_END ()
    }();

    return reflection;
}

#include <Gameplay/SlowdownSingleton.hpp>

#include <StandardLayout/MappingRegistration.hpp>

const SlowdownSingleton::Reflection &SlowdownSingleton::Reflect () noexcept
{
    static Reflection reflection = [] ()
    {
        EMERGENCE_MAPPING_REGISTRATION_BEGIN (SlowdownSingleton)
        EMERGENCE_MAPPING_REGISTER_REGULAR (durationNs)
        EMERGENCE_MAPPING_REGISTER_REGULAR (endTimeNs)
        EMERGENCE_MAPPING_REGISTER_REGULAR (strength)
        EMERGENCE_MAPPING_REGISTRATION_END ()
    }();

    return reflection;
}

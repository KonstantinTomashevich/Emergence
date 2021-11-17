#include <SyntaxSugar/MappingRegistration.hpp>

#include <Time/TimeSingleton.hpp>

const TimeSingleton::Reflection &TimeSingleton::Reflect () noexcept
{
    static Reflection reflection = [] ()
    {
        EMERGENCE_MAPPING_REGISTRATION_BEGIN (TimeSingleton)
        EMERGENCE_MAPPING_REGISTER_REGULAR (fixedFrameIndex)
        EMERGENCE_MAPPING_REGISTER_REGULAR (fixedTimeS)
        EMERGENCE_MAPPING_REGISTER_REGULAR (fixedDurationS)
        EMERGENCE_MAPPING_REGISTER_REGULAR (normalTimeS)
        EMERGENCE_MAPPING_REGISTER_REGULAR (normalDurationS)
        EMERGENCE_MAPPING_REGISTRATION_END ()
    }();

    return reflection;
}

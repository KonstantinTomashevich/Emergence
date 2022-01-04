#include <StandardLayout/MappingRegistration.hpp>

#include <Time/TimeSingleton.hpp>

const TimeSingleton::Reflection &TimeSingleton::Reflect () noexcept
{
    static Reflection reflection = [] ()
    {
        EMERGENCE_MAPPING_REGISTRATION_BEGIN (TimeSingleton)
        EMERGENCE_MAPPING_REGISTER_REGULAR (fixedTimeUs)
        EMERGENCE_MAPPING_REGISTER_REGULAR (fixedDurationS)
        EMERGENCE_MAPPING_REGISTER_REGULAR_ARRAY (targetFixedFrameDurationsS)
        EMERGENCE_MAPPING_REGISTER_REGULAR (fixedStartUs)
        EMERGENCE_MAPPING_REGISTER_NESTED_OBJECT (averageFixedRealDurationS)
        EMERGENCE_MAPPING_REGISTER_REGULAR (normalTimeUs)
        EMERGENCE_MAPPING_REGISTER_REGULAR (normalDurationS)
        EMERGENCE_MAPPING_REGISTER_REGULAR (normalTimeOffsetUs)
        EMERGENCE_MAPPING_REGISTER_NESTED_OBJECT (averageNormalRealDurationS)
        EMERGENCE_MAPPING_REGISTRATION_END ()
    }();

    return reflection;
}

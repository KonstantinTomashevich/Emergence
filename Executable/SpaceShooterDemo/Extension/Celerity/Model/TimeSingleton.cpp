#include <Celerity/Model/TimeSingleton.hpp>

#include <StandardLayout/MappingRegistration.hpp>

namespace Emergence::Celerity
{
const TimeSingleton::Reflection &TimeSingleton::Reflect () noexcept
{
    static Reflection reflection = [] ()
    {
        EMERGENCE_MAPPING_REGISTRATION_BEGIN (TimeSingleton)

        EMERGENCE_MAPPING_REGISTER_REGULAR (normalTimeNs)
        EMERGENCE_MAPPING_REGISTER_REGULAR (realNormalTimeNs)
        EMERGENCE_MAPPING_REGISTER_REGULAR (fixedTimeNs)

        EMERGENCE_MAPPING_REGISTER_REGULAR (normalDurationS)
        EMERGENCE_MAPPING_REGISTER_REGULAR (realNormalDurationS)
        EMERGENCE_MAPPING_REGISTER_REGULAR (fixedDurationS)
        EMERGENCE_MAPPING_REGISTER_REGULAR_ARRAY (targetFixedFrameDurationsS)

        EMERGENCE_MAPPING_REGISTER_REGULAR (timeSpeed)

        EMERGENCE_MAPPING_REGISTER_NESTED_OBJECT (averageFullFrameRealDurationS)

        EMERGENCE_MAPPING_REGISTRATION_END ()
    }();

    return reflection;
}
} // namespace Emergence::Celerity

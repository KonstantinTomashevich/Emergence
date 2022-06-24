#include <Gameplay/MortalitySettingsSingleton.hpp>

#include <StandardLayout/MappingRegistration.hpp>

const MortalitySettingsSingleton::Reflection &MortalitySettingsSingleton::Reflect () noexcept
{
    static Reflection reflection = [] ()
    {
        EMERGENCE_MAPPING_REGISTRATION_BEGIN (MortalitySettingsSingleton)
        EMERGENCE_MAPPING_REGISTER_REGULAR (corpseLifetimeNs)
        EMERGENCE_MAPPING_REGISTRATION_END ()
    }();

    return reflection;
}

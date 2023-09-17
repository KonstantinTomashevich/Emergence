#include <Platformer/Team/TeamConfigurationSingleton.hpp>

#include <StandardLayout/MappingRegistration.hpp>

const TeamConfigurationSingleton::Reflection &TeamConfigurationSingleton::Reflect () noexcept
{
    static const Reflection reflection = [] ()
    {
        EMERGENCE_MAPPING_REGISTRATION_BEGIN (TeamConfigurationSingleton);
        EMERGENCE_MAPPING_REGISTER_REGULAR (playerTeamId);
        EMERGENCE_MAPPING_REGISTRATION_END ();
    }();

    return reflection;
}

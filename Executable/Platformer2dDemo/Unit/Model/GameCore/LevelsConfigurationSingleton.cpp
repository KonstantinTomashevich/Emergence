#include <GameCore/LevelsConfigurationSingleton.hpp>

#include <StandardLayout/MappingRegistration.hpp>

const LevelsConfigurationSingleton::Reflection &LevelsConfigurationSingleton::Reflect () noexcept
{
    static const Reflection reflection = [] ()
    {
        EMERGENCE_MAPPING_REGISTRATION_BEGIN (LevelsConfigurationSingleton);
        EMERGENCE_MAPPING_REGISTER_REGULAR (tutorialLevelName);
        EMERGENCE_MAPPING_REGISTER_REGULAR (campaignLevelPrefix);
        EMERGENCE_MAPPING_REGISTER_REGULAR (campaignLevelCount);
        EMERGENCE_MAPPING_REGISTER_REGULAR (loaded);
        EMERGENCE_MAPPING_REGISTRATION_END ();
    }();

    return reflection;
}

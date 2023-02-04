#include <MainMenuLoading/MainMenuLoadingSingleton.hpp>

#include <StandardLayout/MappingRegistration.hpp>

const MainMenuLoadingSingleton::Reflection &MainMenuLoadingSingleton::Reflect () noexcept
{
    static const Reflection reflection = [] ()
    {
        EMERGENCE_MAPPING_REGISTRATION_BEGIN (MainMenuLoadingSingleton);
        EMERGENCE_MAPPING_REGISTER_REGULAR (levelsConfigurationLoaded);
        EMERGENCE_MAPPING_REGISTER_REGULAR (mainMenuInitialized);
        EMERGENCE_MAPPING_REGISTER_REGULAR (assetsLoaded);
        EMERGENCE_MAPPING_REGISTRATION_END ();
    }();

    return reflection;
}

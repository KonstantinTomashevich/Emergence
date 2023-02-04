#include <MainMenu/MainMenuSingleton.hpp>

#include <StandardLayout/MappingRegistration.hpp>

const MainMenuSingleton::Reflection &MainMenuSingleton::Reflect () noexcept
{
    static const Reflection reflection = [] ()
    {
        EMERGENCE_MAPPING_REGISTRATION_BEGIN (MainMenuSingleton);
        EMERGENCE_MAPPING_REGISTER_REGULAR (uiListenerObjectId);
        EMERGENCE_MAPPING_REGISTRATION_END ();
    }();

    return reflection;
}

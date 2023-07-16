#include <LevelLoading/LevelLoadingSingleton.hpp>

#include <StandardLayout/MappingRegistration.hpp>

const LevelLoadingSingleton::Reflection &LevelLoadingSingleton::Reflect () noexcept
{
    static const Reflection reflection = [] ()
    {
        EMERGENCE_MAPPING_REGISTRATION_BEGIN (LevelLoadingSingleton);
        EMERGENCE_MAPPING_REGISTER_REGULAR (levelName);
        EMERGENCE_MAPPING_REGISTER_REGULAR (state);
        EMERGENCE_MAPPING_REGISTRATION_END ();
    }();

    return reflection;
}

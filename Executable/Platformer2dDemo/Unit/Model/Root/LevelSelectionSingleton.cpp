#include <Root/LevelSelectionSingleton.hpp>

#include <StandardLayout/MappingRegistration.hpp>

const LevelSelectionSingleton::Reflection &LevelSelectionSingleton::Reflect () noexcept
{
    static const Reflection reflection = [] ()
    {
        EMERGENCE_MAPPING_REGISTRATION_BEGIN (LevelSelectionSingleton);
        EMERGENCE_MAPPING_REGISTER_REGULAR (selectedLevelName);
        EMERGENCE_MAPPING_REGISTRATION_END ();
    }();

    return reflection;
}

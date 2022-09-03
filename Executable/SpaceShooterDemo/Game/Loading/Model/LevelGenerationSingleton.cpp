#include <Loading/Model/LevelGenerationSingleton.hpp>

#include <StandardLayout/MappingRegistration.hpp>

const LevelGenerationSingleton::Reflection &LevelGenerationSingleton::Reflect () noexcept
{
    static Reflection reflection = [] ()
    {
        EMERGENCE_MAPPING_REGISTRATION_BEGIN (LevelGenerationSingleton);
        EMERGENCE_MAPPING_REGISTER_REGULAR (finished);
        EMERGENCE_MAPPING_REGISTRATION_END ();
    }();

    return reflection;
}

#include <Loading/Model/InputInitializationSingleton.hpp>

#include <StandardLayout/MappingRegistration.hpp>

const InputInitializationSingleton::Reflection &InputInitializationSingleton::Reflect () noexcept
{
    static Reflection reflection = [] ()
    {
        EMERGENCE_MAPPING_REGISTRATION_BEGIN (InputInitializationSingleton);
        EMERGENCE_MAPPING_REGISTER_REGULAR (finished);
        EMERGENCE_MAPPING_REGISTRATION_END ();
    }();

    return reflection;
}

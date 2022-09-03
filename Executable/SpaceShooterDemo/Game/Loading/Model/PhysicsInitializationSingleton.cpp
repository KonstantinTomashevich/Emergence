#include <Loading/Model/PhysicsInitializationSingleton.hpp>

#include <StandardLayout/MappingRegistration.hpp>

const PhysicsInitializationSingleton::Reflection &PhysicsInitializationSingleton::Reflect () noexcept
{
    static Reflection reflection = [] ()
    {
        EMERGENCE_MAPPING_REGISTRATION_BEGIN (PhysicsInitializationSingleton);
        EMERGENCE_MAPPING_REGISTER_REGULAR (finished);
        EMERGENCE_MAPPING_REGISTRATION_END ();
    }();

    return reflection;
}

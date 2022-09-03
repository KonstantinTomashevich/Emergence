#include <Loading/Model/DynamicsMaterialLoadingSingleton.hpp>

#include <StandardLayout/MappingRegistration.hpp>

const DynamicsMaterialLoadingSingleton::Reflection &DynamicsMaterialLoadingSingleton::Reflect () noexcept
{
    static Reflection reflection = [] ()
    {
        EMERGENCE_MAPPING_REGISTRATION_BEGIN (DynamicsMaterialLoadingSingleton);
        EMERGENCE_MAPPING_REGISTER_REGULAR (finished);
        EMERGENCE_MAPPING_REGISTRATION_END ();
    }();

    return reflection;
}

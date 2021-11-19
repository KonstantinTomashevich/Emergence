#include <Input/NormalInputMappingSingleton.hpp>

#include <SyntaxSugar/MappingRegistration.hpp>

const NormalInputMappingSingleton::Reflection &NormalInputMappingSingleton::Reflect () noexcept
{
    static Reflection reflection = [] ()
    {
        EMERGENCE_MAPPING_REGISTRATION_BEGIN (NormalInputMappingSingleton)
        EMERGENCE_MAPPING_REGISTER_NESTED_OBJECT (inputMapping)
        EMERGENCE_MAPPING_REGISTRATION_END ()
    }();

    return reflection;
}

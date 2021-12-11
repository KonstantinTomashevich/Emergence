#include <Input/FixedInputMappingSingleton.hpp>

#include <SyntaxSugar/MappingRegistration.hpp>

const FixedInputMappingSingleton::Reflection &FixedInputMappingSingleton::Reflect () noexcept
{
    static Reflection reflection = [] ()
    {
        EMERGENCE_MAPPING_REGISTRATION_BEGIN (FixedInputMappingSingleton)
        EMERGENCE_MAPPING_REGISTER_NESTED_OBJECT (inputMapping)
        EMERGENCE_MAPPING_REGISTRATION_END ()
    }();

    return reflection;
}

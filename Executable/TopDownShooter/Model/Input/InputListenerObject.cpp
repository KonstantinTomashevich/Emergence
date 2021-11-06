#include <Input/InputListenerObject.hpp>

#include <SyntaxSugar/MappingRegistration.hpp>

const InputListenerObject::Reflection &InputListenerObject::Reflect () noexcept
{
    static Reflection reflection = [] ()
    {
        EMERGENCE_MAPPING_REGISTRATION_BEGIN (InputListenerObject)
        EMERGENCE_MAPPING_REGISTER_REGULAR (objectId)
        EMERGENCE_MAPPING_REGISTER_NESTED_OBJECT_ARRAY (actions)
        EMERGENCE_MAPPING_REGISTRATION_END ()
    }();

    return reflection;
}

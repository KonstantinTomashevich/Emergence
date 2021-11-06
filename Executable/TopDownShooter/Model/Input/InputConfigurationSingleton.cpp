#include <Input/InputConfigurationSingleton.hpp>

#include <SyntaxSugar/MappingRegistration.hpp>

const InputConfigurationSingleton::Reflection &InputConfigurationSingleton::Reflect () noexcept
{
    static Reflection reflection = [] ()
    {
        EMERGENCE_MAPPING_REGISTRATION_BEGIN (InputConfigurationSingleton)
        EMERGENCE_MAPPING_REGISTER_REGULAR (readyForRouting)
        EMERGENCE_MAPPING_REGISTER_REGULAR (listenerObjectId)
        EMERGENCE_MAPPING_REGISTRATION_END ()
    }();

    return reflection;
}

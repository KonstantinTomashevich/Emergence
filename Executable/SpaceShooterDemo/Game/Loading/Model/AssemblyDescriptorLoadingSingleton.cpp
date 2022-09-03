#include <Loading/Model/AssemblyDescriptorLoadingSingleton.hpp>

#include <StandardLayout/MappingRegistration.hpp>

const AssemblyDescriptorLoadingSingleton::Reflection &AssemblyDescriptorLoadingSingleton::Reflect () noexcept
{
    static Reflection reflection = [] ()
    {
        EMERGENCE_MAPPING_REGISTRATION_BEGIN (AssemblyDescriptorLoadingSingleton);
        EMERGENCE_MAPPING_REGISTER_REGULAR (started);
        EMERGENCE_MAPPING_REGISTER_REGULAR (finished);
        EMERGENCE_MAPPING_REGISTRATION_END ();
    }();

    return reflection;
}

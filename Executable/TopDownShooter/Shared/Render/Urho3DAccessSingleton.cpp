#include <Render/Urho3DAccessSingleton.hpp>

#include <StandardLayout/MappingRegistration.hpp>

Urho3DAccessSingleton::Reflection &Urho3DAccessSingleton::Reflect () noexcept
{
    static Reflection reflection = [] ()
    {
        EMERGENCE_MAPPING_REGISTRATION_BEGIN (Urho3DAccessSingleton)
        EMERGENCE_MAPPING_REGISTRATION_END ()
    }();

    return reflection;
}

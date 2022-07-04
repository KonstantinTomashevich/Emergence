#include <Render/Urho3DSceneSingleton.hpp>

#include <StandardLayout/MappingRegistration.hpp>

Urho3DSceneSingleton::Urho3DSceneSingleton () noexcept = default;

Urho3DSceneSingleton::~Urho3DSceneSingleton () noexcept
{
    delete scene;
}

const Urho3DSceneSingleton::Reflection &Urho3DSceneSingleton::Reflect () noexcept
{
    static Reflection reflection = [] ()
    {
        EMERGENCE_MAPPING_REGISTRATION_BEGIN (Urho3DSceneSingleton);
        EMERGENCE_MAPPING_REGISTER_REGULAR (scene);
        EMERGENCE_MAPPING_REGISTRATION_END ();
    }();

    return reflection;
}

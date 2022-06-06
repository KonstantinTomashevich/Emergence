#include <Render/Urho3DNodeComponent.hpp>

#include <StandardLayout/MappingRegistration.hpp>

Urho3DNodeComponent::Urho3DNodeComponent () noexcept = default;

Urho3DNodeComponent::~Urho3DNodeComponent () noexcept = default;

const Urho3DNodeComponent::Reflection &Urho3DNodeComponent::Reflect () noexcept
{
    static Reflection reflection = [] ()
    {
        EMERGENCE_MAPPING_REGISTRATION_BEGIN (Urho3DNodeComponent)
        EMERGENCE_MAPPING_REGISTER_REGULAR (objectId)
        EMERGENCE_MAPPING_REGISTER_POINTER_AS_REGULAR (implementationHandle)
        EMERGENCE_MAPPING_REGISTRATION_END ()
    }();

    return reflection;
}

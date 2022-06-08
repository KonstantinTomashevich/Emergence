#include <Render/LightComponent.hpp>

#include <StandardLayout/MappingRegistration.hpp>

LightComponent::LightComponent () noexcept = default;

LightComponent::~LightComponent () noexcept = default;

const LightComponent::Reflection &LightComponent::Reflect () noexcept
{
    static Reflection reflection = [] ()
    {
        EMERGENCE_MAPPING_REGISTRATION_BEGIN (LightComponent)
        EMERGENCE_MAPPING_REGISTER_REGULAR (objectId)
        EMERGENCE_MAPPING_REGISTER_REGULAR (lightId)
        EMERGENCE_MAPPING_REGISTER_ENUM_AS_REGULAR (type)
        EMERGENCE_MAPPING_REGISTER_REGULAR (enabled)
        EMERGENCE_MAPPING_REGISTER_NESTED_OBJECT (color)
        EMERGENCE_MAPPING_REGISTER_REGULAR (range)
        EMERGENCE_MAPPING_REGISTER_REGULAR (spotFieldOfViewRad)
        EMERGENCE_MAPPING_REGISTER_REGULAR (spotAspectRatio)
        EMERGENCE_MAPPING_REGISTER_POINTER_AS_REGULAR (implementationHandle)
        EMERGENCE_MAPPING_REGISTRATION_END ()
    }();

    return reflection;
}

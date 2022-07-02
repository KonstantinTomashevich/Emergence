#include <Render/StaticModelComponent.hpp>

#include <StandardLayout/MappingRegistration.hpp>

StaticModelComponent::StaticModelComponent () noexcept = default;

StaticModelComponent::~StaticModelComponent () noexcept = default;

const StaticModelComponent::Reflection &StaticModelComponent::Reflect () noexcept
{
    static Reflection reflection = [] ()
    {
        EMERGENCE_MAPPING_REGISTRATION_BEGIN (StaticModelComponent);
        EMERGENCE_MAPPING_REGISTER_REGULAR (objectId);
        EMERGENCE_MAPPING_REGISTER_REGULAR (modelId);
        EMERGENCE_MAPPING_REGISTER_UNIQUE_STRING (modelName);
        EMERGENCE_MAPPING_REGISTER_UNIQUE_STRING_ARRAY (materialNames);
        EMERGENCE_MAPPING_REGISTER_POINTER_AS_REGULAR (implementationHandle);
        EMERGENCE_MAPPING_REGISTRATION_END ();
    }();

    return reflection;
}

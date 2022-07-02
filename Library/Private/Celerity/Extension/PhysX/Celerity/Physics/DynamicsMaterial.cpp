#include <Celerity/Physics/DynamicsMaterial.hpp>

#include <StandardLayout/MappingRegistration.hpp>

namespace Emergence::Celerity
{
DynamicsMaterial::DynamicsMaterial () noexcept = default;

DynamicsMaterial::~DynamicsMaterial () noexcept = default;

const DynamicsMaterial::Reflection &DynamicsMaterial::Reflect () noexcept
{
    static Reflection reflection = [] ()
    {
        EMERGENCE_MAPPING_REGISTRATION_BEGIN (DynamicsMaterial);
        EMERGENCE_MAPPING_REGISTER_UNIQUE_STRING (id);
        EMERGENCE_MAPPING_REGISTER_REGULAR (dynamicFriction);
        EMERGENCE_MAPPING_REGISTER_REGULAR (staticFriction);
        EMERGENCE_MAPPING_REGISTER_REGULAR (enableFriction);
        EMERGENCE_MAPPING_REGISTER_REGULAR (restitution);
        EMERGENCE_MAPPING_REGISTER_REGULAR (density);
        EMERGENCE_MAPPING_REGISTER_POINTER_AS_REGULAR (implementationHandle);
        EMERGENCE_MAPPING_REGISTRATION_END ();
    }();

    return reflection;
}
} // namespace Emergence::Celerity

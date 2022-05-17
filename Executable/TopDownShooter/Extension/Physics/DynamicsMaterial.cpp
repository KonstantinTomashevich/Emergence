#include <Physics/DynamicsMaterial.hpp>

#include <PxMaterial.h>

#include <StandardLayout/MappingRegistration.hpp>

namespace Emergence::Physics
{
DynamicsMaterial::~DynamicsMaterial () noexcept
{
    if (implementationHandle)
    {
        static_cast<physx::PxMaterial *> (implementationHandle)->release ();
    }
}

const DynamicsMaterial::Reflection &DynamicsMaterial::Reflect () noexcept
{
    static Reflection reflection = [] ()
    {
        EMERGENCE_MAPPING_REGISTRATION_BEGIN (DynamicsMaterial)
        EMERGENCE_MAPPING_REGISTER_REGULAR (id)
        EMERGENCE_MAPPING_REGISTER_REGULAR (dynamicFriction)
        EMERGENCE_MAPPING_REGISTER_REGULAR (staticFriction)
        EMERGENCE_MAPPING_REGISTER_REGULAR (enableFriction)
        EMERGENCE_MAPPING_REGISTER_REGULAR (restitution)
        EMERGENCE_MAPPING_REGISTER_REGULAR (density)
        EMERGENCE_MAPPING_REGISTRATION_END ()
    }();

    return reflection;
}
} // namespace Emergence::Physics

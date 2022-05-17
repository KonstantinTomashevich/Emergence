#include <Physics/CollisionShapeComponent.hpp>

#include <PxShape.h>

#include <StandardLayout/MappingRegistration.hpp>

namespace Emergence::Physics
{
CollisionShapeComponent::~CollisionShapeComponent () noexcept
{
    if (implementationHandle)
    {
        static_cast<physx::PxShape *> (implementationHandle)->release ();
    }
}

const CollisionShapeComponent::Reflection &CollisionShapeComponent::Reflect () noexcept
{
    static Reflection reflection = [] ()
    {
        EMERGENCE_MAPPING_REGISTRATION_BEGIN (CollisionShapeComponent)
        EMERGENCE_MAPPING_REGISTER_NESTED_OBJECT (geometry)
        EMERGENCE_MAPPING_REGISTER_NESTED_OBJECT (translation)
        EMERGENCE_MAPPING_REGISTER_NESTED_OBJECT (rotation)
        EMERGENCE_MAPPING_REGISTER_REGULAR (materialId)
        EMERGENCE_MAPPING_REGISTER_REGULAR (enabled)
        EMERGENCE_MAPPING_REGISTER_REGULAR (trigger)
        EMERGENCE_MAPPING_REGISTER_REGULAR (visibleToWorldQueries)
        EMERGENCE_MAPPING_REGISTER_REGULAR (collisionGroup)
        EMERGENCE_MAPPING_REGISTRATION_END ()
    }();

    return reflection;
}
} // namespace Emergence::Physics

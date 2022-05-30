#include <Physics/CollisionShapeComponent.hpp>

#include <PxRigidActor.h>
#include <PxShape.h>

#include <StandardLayout/MappingRegistration.hpp>

namespace Emergence::Physics
{
CollisionShapeComponent::CollisionShapeComponent () noexcept = default;

CollisionShapeComponent::~CollisionShapeComponent () noexcept = default;

const CollisionShapeComponent::Reflection &CollisionShapeComponent::Reflect () noexcept
{
    static Reflection reflection = [] ()
    {
        EMERGENCE_MAPPING_REGISTRATION_BEGIN (CollisionShapeComponent)
        EMERGENCE_MAPPING_REGISTER_REGULAR (shapeId)
        EMERGENCE_MAPPING_REGISTER_REGULAR (objectId)
        EMERGENCE_MAPPING_REGISTER_UNIQUE_STRING (materialId)
        EMERGENCE_MAPPING_REGISTER_NESTED_OBJECT (geometry)
        EMERGENCE_MAPPING_REGISTER_NESTED_OBJECT (translation)
        EMERGENCE_MAPPING_REGISTER_NESTED_OBJECT (rotation)
        EMERGENCE_MAPPING_REGISTER_REGULAR (enabled)
        EMERGENCE_MAPPING_REGISTER_REGULAR (trigger)
        EMERGENCE_MAPPING_REGISTER_REGULAR (visibleToWorldQueries)
        EMERGENCE_MAPPING_REGISTER_REGULAR (collisionGroup)
        EMERGENCE_MAPPING_REGISTER_POINTER_AS_REGULAR (implementationHandle)
        EMERGENCE_MAPPING_REGISTRATION_END ()
    }();

    return reflection;
}
} // namespace Emergence::Physics

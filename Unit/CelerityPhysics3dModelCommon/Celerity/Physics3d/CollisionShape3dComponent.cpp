#include <Celerity/Physics3d/CollisionShape3dComponent.hpp>

#include <StandardLayout/MappingRegistration.hpp>

namespace Emergence::Celerity
{
CollisionShape3dComponent::CollisionShape3dComponent () noexcept = default;

CollisionShape3dComponent::~CollisionShape3dComponent () noexcept = default;

const CollisionShape3dComponent::Reflection &CollisionShape3dComponent::Reflect () noexcept
{
    static Reflection reflection = [] ()
    {
        EMERGENCE_MAPPING_REGISTRATION_BEGIN (CollisionShape3dComponent);
        EMERGENCE_MAPPING_REGISTER_REGULAR (shapeId);
        EMERGENCE_MAPPING_REGISTER_REGULAR (objectId);
        EMERGENCE_MAPPING_REGISTER_REGULAR (rotation);
        EMERGENCE_MAPPING_REGISTER_REGULAR (translation);
        EMERGENCE_MAPPING_REGISTER_REGULAR (geometry);
        EMERGENCE_MAPPING_REGISTER_REGULAR (materialId);
        EMERGENCE_MAPPING_REGISTER_REGULAR (enabled);
        EMERGENCE_MAPPING_REGISTER_REGULAR (trigger);
        EMERGENCE_MAPPING_REGISTER_REGULAR (visibleToWorldQueries);
        EMERGENCE_MAPPING_REGISTER_REGULAR (sendContactEvents);
        EMERGENCE_MAPPING_REGISTER_REGULAR (collisionGroup);
        EMERGENCE_MAPPING_REGISTER_REGULAR (implementationHandle);
        EMERGENCE_MAPPING_REGISTRATION_END ();
    }();

    return reflection;
}
} // namespace Emergence::Celerity

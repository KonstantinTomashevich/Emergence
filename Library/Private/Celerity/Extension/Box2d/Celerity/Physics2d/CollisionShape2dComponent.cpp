#include <Celerity/Physics2d/CollisionShape2dComponent.hpp>

#include <StandardLayout/MappingRegistration.hpp>

namespace Emergence::Celerity
{
CollisionShape2dComponent::CollisionShape2dComponent () noexcept = default;

CollisionShape2dComponent::~CollisionShape2dComponent () noexcept = default;

const CollisionShape2dComponent::Reflection &CollisionShape2dComponent::Reflect () noexcept
{
    static Reflection reflection = [] ()
    {
        EMERGENCE_MAPPING_REGISTRATION_BEGIN (CollisionShape2dComponent);
        EMERGENCE_MAPPING_REGISTER_REGULAR (shapeId);
        EMERGENCE_MAPPING_REGISTER_REGULAR (objectId);
        EMERGENCE_MAPPING_REGISTER_REGULAR (translation);
        EMERGENCE_MAPPING_REGISTER_REGULAR (rotation);
        EMERGENCE_MAPPING_REGISTER_REGULAR (geometry);
        EMERGENCE_MAPPING_REGISTER_REGULAR (materialId);
        EMERGENCE_MAPPING_REGISTER_REGULAR (trigger);
        EMERGENCE_MAPPING_REGISTER_REGULAR (visibleToWorldQueries);
        EMERGENCE_MAPPING_REGISTER_REGULAR (maintainCollisionContacts);
        EMERGENCE_MAPPING_REGISTER_REGULAR (collisionGroup);
        EMERGENCE_MAPPING_REGISTER_REGULAR (implementationHandle);
        EMERGENCE_MAPPING_REGISTRATION_END ();
    }();

    return reflection;
}
} // namespace Emergence::Celerity

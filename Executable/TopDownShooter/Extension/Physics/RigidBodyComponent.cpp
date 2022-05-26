#include <Physics/RigidBodyComponent.hpp>

#include <PxRigidBody.h>

#include <StandardLayout/MappingRegistration.hpp>

namespace Emergence::Physics
{
RigidBodyComponent::RigidBodyComponent () noexcept = default;

// We can not release bodies from destructor, because it will affect shapes and might cause race condition.
// Therefore, we are deleting these implementation handles inside simulation callback.
RigidBodyComponent::~RigidBodyComponent () noexcept = default;

const RigidBodyComponent::Reflection &RigidBodyComponent::Reflect () noexcept
{
    static Reflection reflection = [] ()
    {
        EMERGENCE_MAPPING_REGISTRATION_BEGIN (RigidBodyComponent)
        EMERGENCE_MAPPING_REGISTER_REGULAR (objectId)
        EMERGENCE_MAPPING_REGISTER_ENUM_AS_REGULAR (type)
        EMERGENCE_MAPPING_REGISTER_REGULAR (linearDamping)
        EMERGENCE_MAPPING_REGISTER_REGULAR (angularDamping)
        EMERGENCE_MAPPING_REGISTER_REGULAR (continuousCollisionDetection)
        EMERGENCE_MAPPING_REGISTER_REGULAR (affectedByGravity)
        EMERGENCE_MAPPING_REGISTER_REGULAR (manipulatedOutsideOfSimulation)
        EMERGENCE_MAPPING_REGISTER_REGULAR (sendContactEvents)
        EMERGENCE_MAPPING_REGISTER_NESTED_OBJECT (linearVelocity)
        EMERGENCE_MAPPING_REGISTER_NESTED_OBJECT (angularVelocity)
        EMERGENCE_MAPPING_REGISTER_NESTED_OBJECT (additiveLinearImpulse)
        EMERGENCE_MAPPING_REGISTER_NESTED_OBJECT (additiveAngularImpulse)
        EMERGENCE_MAPPING_REGISTER_POINTER_AS_REGULAR (implementationHandle)
        EMERGENCE_MAPPING_REGISTRATION_END ()
    }();

    return reflection;
}
} // namespace Emergence::Physics

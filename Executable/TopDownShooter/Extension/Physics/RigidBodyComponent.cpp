#include <Physics/RigidBodyComponent.hpp>

#include <PxRigidBody.h>

#include <StandardLayout/MappingRegistration.hpp>

namespace Emergence::Physics
{
RigidBodyComponent::RigidBodyComponent () noexcept = default;

RigidBodyComponent::~RigidBodyComponent () noexcept
{
    if (implementationHandle)
    {
        // TODO: Can we really release bodies? It affects shapes, therefore it is not thread safe.
        static_cast<physx::PxRigidBody *> (implementationHandle)->release ();
    }
}

const RigidBodyComponent::Reflection &RigidBodyComponent::Reflect () noexcept
{
    static Reflection reflection = [] ()
    {
        EMERGENCE_MAPPING_REGISTRATION_BEGIN (RigidBodyComponent)
        EMERGENCE_MAPPING_REGISTER_REGULAR (objectId)
        EMERGENCE_MAPPING_REGISTER_ENUM_AS_REGULAR (type)
        EMERGENCE_MAPPING_REGISTER_REGULAR (continuousCollisionDetection)
        EMERGENCE_MAPPING_REGISTER_REGULAR (affectedByGravity)
        EMERGENCE_MAPPING_REGISTER_NESTED_OBJECT (linearVelocity)
        EMERGENCE_MAPPING_REGISTER_NESTED_OBJECT (angularVelocity)
        EMERGENCE_MAPPING_REGISTER_NESTED_OBJECT (additiveLinearImpulse)
        EMERGENCE_MAPPING_REGISTER_NESTED_OBJECT (additiveAngularImpulse)
        EMERGENCE_MAPPING_REGISTER_REGULAR (linearDamping)
        EMERGENCE_MAPPING_REGISTER_REGULAR (angularDamping)
        EMERGENCE_MAPPING_REGISTER_POINTER_AS_REGULAR (implementationHandle)
        EMERGENCE_MAPPING_REGISTRATION_END ()
    }();

    return reflection;
}
} // namespace Emergence::Physics

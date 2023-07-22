#include <Celerity/Physics3d/RigidBody3dComponent.hpp>

#include <StandardLayout/MappingRegistration.hpp>

namespace Emergence::Celerity
{
RigidBody3dComponent::RigidBody3dComponent () noexcept = default;

RigidBody3dComponent::~RigidBody3dComponent () noexcept = default;

const RigidBody3dComponent::Reflection &RigidBody3dComponent::Reflect () noexcept
{
    static Reflection reflection = [] ()
    {
        EMERGENCE_MAPPING_REGISTRATION_BEGIN (RigidBody3dComponent);
        EMERGENCE_MAPPING_REGISTER_REGULAR (objectId);
        EMERGENCE_MAPPING_REGISTER_REGULAR (type);
        EMERGENCE_MAPPING_REGISTER_REGULAR (linearDamping);
        EMERGENCE_MAPPING_REGISTER_REGULAR (angularDamping);
        EMERGENCE_MAPPING_REGISTER_REGULAR (continuousCollisionDetection);
        EMERGENCE_MAPPING_REGISTER_REGULAR (affectedByGravity);
        EMERGENCE_MAPPING_REGISTER_REGULAR (manipulatedOutsideOfSimulation);
        EMERGENCE_MAPPING_REGISTER_REGULAR (lockFlags);
        EMERGENCE_MAPPING_REGISTER_REGULAR (linearVelocity);
        EMERGENCE_MAPPING_REGISTER_REGULAR (angularVelocity);
        EMERGENCE_MAPPING_REGISTER_REGULAR (additiveLinearImpulse);
        EMERGENCE_MAPPING_REGISTER_REGULAR (additiveAngularImpulse);
        EMERGENCE_MAPPING_REGISTER_REGULAR (implementationHandle);
        EMERGENCE_MAPPING_REGISTRATION_END ();
    }();

    return reflection;
}
} // namespace Emergence::Celerity

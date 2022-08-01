#pragma once

#include <API/Common/Shortcuts.hpp>

#include <Celerity/Standard/UniqueId.hpp>

#include <Math/Vector3f.hpp>

namespace Emergence::Celerity
{
/// \brief Enumerates supported rigid body types.
enum class RigidBodyType
{
    /// \brief Static bodies are not affected by any forces or velocities
    ///        and their transform can not be changed by physics engine.
    STATIC = 0u,

    /// \brief Kinematic bodies are affected by velocities, but not by forces.
    /// \details Collisions do not stop kinematic bodies movement and can not change movement direction.
    KINEMATIC,

    /// \brief Dynamic bodies are affected both by velocities and forces.
    DYNAMIC
};

/// \brief Represents rigid body attached to an object.
struct RigidBodyComponent final
{
    /// \brief Forbids body to change its translation on X axis.
    /// \see ::lockFlags
    static constexpr uint8_t LOCK_LINEAR_X = 1u << 0u;

    /// \brief Forbids body to change its translation on Y axis.
    /// \see ::lockFlags
    static constexpr uint8_t LOCK_LINEAR_Y = 1u << 1u;

    /// \brief Forbids body to change its translation on Z axis.
    /// \see ::lockFlags
    static constexpr uint8_t LOCK_LINEAR_Z = 1u << 2u;

    /// \brief Forbids body to change its rotation over X axis.
    /// \see ::lockFlags
    static constexpr uint8_t LOCK_ANGULAR_X = 1u << 3u;

    /// \brief Forbids body to change its rotation over Y axis.
    /// \see ::lockFlags
    static constexpr uint8_t LOCK_ANGULAR_Y = 1u << 4u;

    /// \brief Forbids body to change its rotation over Z axis.
    /// \see ::lockFlags
    static constexpr uint8_t LOCK_ANGULAR_Z = 1u << 5u;

    EMERGENCE_STATIONARY_DATA_TYPE (RigidBodyComponent);

    /// \brief Id of an object with Transform3dComponent to which rigid body is attached.
    UniqueId objectId = INVALID_UNIQUE_ID;

    /// \brief Type of this rigid body.
    /// \invariant Can not be changed after body creation.
    RigidBodyType type = RigidBodyType::STATIC;

    /// \brief Damping coefficient for linear velocity.
    float linearDamping = 0.0f;

    /// \brief Damping coefficient for angular velocity.
    float angularDamping = 0.05f;

    /// \brief Whether continuous collision detection should be enabled for this object.
    /// \details Only supported for dynamic bodies. Costly from performance point of view.
    bool continuousCollisionDetection = false;

    /// \brief Whether object is affected by gravity.
    /// \details Ignored if body is not dynamic.
    bool affectedByGravity = true;

    /// \brief Whether rigid body transform, flags, attributes, velocity or impulse
    ///        can be edited outside of physics simulation after object creation.
    /// \details Can be turned on and off at any time. Use it to dynamically enable
    ///          observation when needed and disable when it is no longer needed.
    bool manipulatedOutsideOfSimulation = false;

    /// \brief Contains combination of transform-locking flags.
    uint8_t lockFlags = 0u;

    /// \brief Body linear velocity in world coordinates.
    Math::Vector3f linearVelocity = Math::Vector3f::ZERO;

    /// \brief Body angular velocity in world coordinates.
    Math::Vector3f angularVelocity = Math::Vector3f::ZERO;

    /// \brief Linear impulse in world coordinates, that will be applied to body during next simulation step.
    Math::Vector3f additiveLinearImpulse = Math::Vector3f::ZERO;

    /// \brief Angular impulse in world coordinates, that will be applied to body during next simulation step.
    Math::Vector3f additiveAngularImpulse = Math::Vector3f::ZERO;

    /// \brief Pointer to implementation-specific object.
    void *implementationHandle = nullptr;

    struct Reflection final
    {
        StandardLayout::FieldId objectId;
        StandardLayout::FieldId type;
        StandardLayout::FieldId linearDamping;
        StandardLayout::FieldId angularDamping;
        StandardLayout::FieldId continuousCollisionDetection;
        StandardLayout::FieldId affectedByGravity;
        StandardLayout::FieldId manipulatedOutsideOfSimulation;
        StandardLayout::FieldId lockFlags;
        StandardLayout::FieldId linearVelocity;
        StandardLayout::FieldId angularVelocity;
        StandardLayout::FieldId additiveLinearImpulse;
        StandardLayout::FieldId additiveAngularImpulse;
        StandardLayout::FieldId implementationHandle;
        StandardLayout::Mapping mapping;
    };

    static const Reflection &Reflect () noexcept;
};
} // namespace Emergence::Celerity
